#include "../System/AutoPtr.h"
#include "../Inst/BoolOpSeq.h"
#include "Codegen_C.h"
#include <limits>

Codegen_C::Codegen_C() : on( &main_os ), os( &main_os ) {
    disp_inst_graph = false;
    disp_inst_graph_wo_phi = false;

    add_include( "<stdint.h>" );
    add_include( "<stdlib.h>" );
    add_include( "<time.h>" );

    add_prel( "#pragma cpp_flag -std=c++11" );
}

void Codegen_C::add_include( String name ) {
    includes.insert( name );
}

void Codegen_C::add_prel( String data ) {
    preliminaries.push_back_unique( data );
}

void Codegen_C::add_type( Type *type ) {
    types.insert( type );
}

void Codegen_C::write_to( Stream &out ) {
    on.nsp = 4;
    make_code();

    // include
    for( String inc : includes )
        out << "#include " << inc << "\n";

    // types
    //    std::map<Type *,Vec<OutReg *> > orbt;
    //    for( int i = 0; i < out_regs.size(); ++i )
    //        orbt[ out_regs[ i ].type ] << &out_regs[ i ];
    //    for( Type *type : types )
    //        orbt[ type ];

    //    for( auto iter : orbt )
    //        iter.first->write_C_decl( out );

    // preliminaries
    for( String prel : preliminaries )
        out << prel << "\n";

    //
    out << "int main() {\n";
    out << "    srand(time(0));\n";

    // out_regs
    //    for( auto iter : orbt ) {
    //        if ( iter.second.size() ) {
    //            out << "    " << *iter.first << " ";
    //            for( int n = 0; n < iter.second.size(); ++n ) {
    //                if ( n )
    //                    out << ", ";
    //                out << *iter.second[ n ];
    //            }
    //            out << ";\n";
    //        }
    //    }

    out << main_os.str();
    out << "}\n";
}

void Codegen_C::exec() {
    TODO;
}

static Expr cloned( const Expr &val, Vec<Expr> &created ) {
    val->clone( created );
    return reinterpret_cast<Inst *>( val->op_mp );
}

void get_front( Vec<Expr> &front, Expr inst ) {
    if ( inst->op_id == inst->cur_op_id )
        return;
    inst->op_id = inst->cur_op_id;

    int nb_id = 0;
    for( Expr ch : inst->inp )
        nb_id += not ch->when->always( false );
    for( Expr ch : inst->dep )
        nb_id += not ch->when->always( false );

    if ( nb_id ) {
        for( Expr ch : inst->inp )
            if ( not ch->when->always( false ) )
                get_front( front, ch );
        for( Expr ch : inst->dep )
            if ( not ch->when->always( false ) )
                get_front( front, ch );
    } else
        front << inst;
}

// check if all the children have already been scheduled
static bool ready_to_be_scheduled( Expr inst ) {
    // already in the front ?
    if ( inst->op_id >= Inst::cur_op_id - 1 )
        return false;

    // not computed ?
    for( const Expr &ch : inst->inp )
        if ( ch->op_id < Inst::cur_op_id and not ch->when->always( false ) )
            return false;
    for( const Expr &ch : inst->dep )
        if ( ch->op_id < Inst::cur_op_id and not ch->when->always( false ) )
            return false;

    // ok
    return true;
}

static BoolOpSeq anded( const Vec<BoolOpSeq> &cond_stack ) {
    BoolOpSeq res;
    for( const BoolOpSeq &b : cond_stack )
        res = res and b;
    return res;
}

struct CC_SeqItem {
    virtual ~CC_SeqItem() {}
    virtual void write( Codegen_C *cc ) = 0;
};
struct CC_SeqItemExpr : CC_SeqItem {
    CC_SeqItemExpr( Expr expr ) : expr( expr ) {}
    virtual ~CC_SeqItemExpr() {}
    virtual void write( Codegen_C *cc ) {
        cc->on.write_beg();
        expr->write_dot( *cc->os );
        cc->on.write_end();
    }
    Expr expr;
};

struct CC_SeqItemBlock : CC_SeqItem {
    CC_SeqItemBlock( CC_SeqItemBlock *parent ) : parent( parent ), cur_seq( 0 ) {}
    virtual ~CC_SeqItemBlock() {}
    virtual void write( Codegen_C *cc ) {
        if ( parent ) {
            cond.write_to_stream( cc, cc->on.write_beg() << "if ( ", -1 );
            cc->on.write_end( " ) {" );
            cc->on.nsp += 4;
        }

        for( int i = 0; i < seq[ 0 ].size(); ++i ) {
            seq[ 0 ][ i ]->write( cc );
        }

        if ( parent ) {
            if ( seq[ 1 ].size() ) {
                cc->on.nsp -= 4;
                cc->on << "} else {";
                cc->on.nsp += 4;

                for( int i = 0; i < seq[ 1 ].size(); ++i ) {
                    seq[ 1 ][ i ]->write( cc );
                }
            }

            cc->on.nsp -= 4;
            cc->on << "}";
        }
    }
    SplittedVec<AutoPtr<CC_SeqItem>,8> seq[ 2 ];
    CC_SeqItemBlock *parent;
    BoolOpSeq cond;
    int cur_seq;
};

void Codegen_C::make_code() {
    // a clone of the whole hierarchy
    ++Inst::cur_op_id;
    Vec<Expr> out, created;
    for( Expr inst : fresh )
        out << cloned( inst, created );

    if ( disp_inst_graph )
        Inst::display_graph( out );

    // update inst->when
    for( Expr inst : out )
        inst->update_when( BoolOpSeq( true ) );

    // replication of inlined instructions

    //

    // scheduling
    ++Inst::cur_op_id;
    Vec<Expr> front;
    for( Expr inst : out )
        get_front( front, inst );
    ++Inst::cur_op_id;
    for( Expr inst : front )
        inst->op_id = Inst::cur_op_id;

    CC_SeqItemBlock main_block( 0 );
    CC_SeqItemBlock *cur_block = &main_block;

    Vec<BoolOpSeq> cond_stack;
    cond_stack << true;
    ++Inst::cur_op_id;
    while ( front.size() ) {
        Expr inst;
        // try to find an instruction with the same condition set or an inst the is not going to write anything
        BoolOpSeq cur_cond = anded( cond_stack );
        for( int i = 0; i < front.size(); ++i ) {
            if ( *front[ i ]->when == cur_cond ) {
                inst = front[ i ];
                front.remove_unordered( i );
                break;
            }
        }
        // try to find an instruction with few changes on conditions
        if ( not inst ) {
            int best_score = std::numeric_limits<int>::max(), best_index = -1;
            int best_nb_close = 0, best_nb_else = 0;
            Vec<BoolOpSeq> best_cond_stack;
            BoolOpSeq best_nc;

            for( int i = 0; i < front.size(); ++i ) {
                BoolOpSeq &cond = *front[ i ]->when;
                Vec<BoolOpSeq> trial_cond_stack = cond_stack;
                int nb_close = 0, nb_else = 0;

                BoolOpSeq poped; // the last cond
                if ( cond.always( true ) ) {
                    nb_close += trial_cond_stack.size();
                    trial_cond_stack.resize( 1 );
                } else {
                    while ( not cond.imply( anded( trial_cond_stack ) ) ) {
                        poped = trial_cond_stack.pop_back_val();
                        if ( poped.or_seq.size() )
                            ++nb_close;
                    }
                }
                BoolOpSeq nc = cond - anded( trial_cond_stack );

                if ( nc.imply( not poped ) ) {
                    trial_cond_stack << not poped;
                    nc = nc - not poped;
                    ++nb_else;
                }
                trial_cond_stack << nc;

                int score = nb_close * 10000 + nb_else + 2 * nc.nb_sub_conds();
                if ( best_score > score ) {
                    best_cond_stack = trial_cond_stack;
                    best_nb_close = nb_close;
                    best_nb_else = nb_else;
                    best_score = score;
                    best_index = i;
                    best_nc = nc;
                }
            }
            inst = front[ best_index ];
            cond_stack = best_cond_stack;
            front.remove_unordered( best_index );

            for( ; best_nb_close; --best_nb_close )
                cur_block = cur_block->parent;
            if ( best_nb_else )
                cur_block->cur_seq = 1;
            if ( best_nb_else or best_nc.or_seq.size() ) {
                CC_SeqItemBlock *new_block = new CC_SeqItemBlock( cur_block );
                cur_block->seq[ cur_block->cur_seq ] << new_block;
                new_block->cond = best_nc;
            }
        }

        cur_block->seq[ cur_block->cur_seq ] << new CC_SeqItemExpr( inst );
        inst->op_id = Inst::cur_op_id; // say that it's done
        cur_cond = inst->when;

        // parents
        for( Inst::Parent &p : inst->par ) {
            if ( ready_to_be_scheduled( p.inst ) ) {
                p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                front << p.inst;
            }
        }
    }

    main_block.write( this );
}



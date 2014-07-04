#include "../Inst/CppRegConstraint.h"
#include "../System/AutoPtr.h"
#include "../Inst/BoolOpSeq.h"
#include "../Inst/Type.h"
#include "CC_SeqItemExpr.h"
#include "CC_SeqItemIf.h"
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

CppOutReg *Codegen_C::new_out_reg( Type *type ) {
    return out_regs.push_back( type, out_regs.size() );
}

void Codegen_C::write_out( Expr expr ) {
    if ( expr->out_reg )
        *os << "R" << expr->out_reg->num;
    else
        *os << "no_out_reg";
}

void Codegen_C::write( Type *type ) {
    *os << *type;
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

void Codegen_C::scheduling( CC_SeqItemBlock *cur_block, Vec<Expr> out ) {
    // update inst->when
    for( Expr inst : out )
        inst->update_when( BoolOpSeq() );

    // get the front
    ++Inst::cur_op_id;
    Vec<Expr> front;
    for( Expr inst : out )
        get_front( front, inst );
    ++Inst::cur_op_id;
    for( Expr inst : front )
        inst->op_id = Inst::cur_op_id;

    // go to the roots
    Vec<CC_SeqItemExpr *> seq_expr;
    Vec<BoolOpSeq> cond_stack;
    cond_stack << BoolOpSeq();
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
                    nb_close = trial_cond_stack.size() - 1;
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
                    --nb_close;
                    ++nb_else;
                }
                if ( nc.or_seq.size() )
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
                cur_block = cur_block->parent_block;
            if ( best_nb_else )
                cur_block = cur_block->sibling;
            if ( best_nc.or_seq.size() ) {
                CC_SeqItemIf *new_block = new CC_SeqItemIf( cur_block );
                cur_block->seq << new_block;
                new_block->cond = best_nc;
                cur_block = &new_block->seq[ 0 ];
            }
        }

        // the new CC_SeqItemExpr
        CC_SeqItemExpr *ne = new CC_SeqItemExpr( inst, cur_block );
        inst->op_id = Inst::cur_op_id; // say that it's done
        cur_cond = *inst->when;
        cur_block->seq << ne;
        seq_expr << ne;

        // parents
        for( Inst::Parent &p : inst->par ) {
            if ( ready_to_be_scheduled( p.inst ) ) {
                p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                front << p.inst;
            }
        }
    }

    // delayed operation (ext blocks)
    for( CC_SeqItemExpr *ne : seq_expr ) {
        // schedule sub block (ext instructions)
        int s = ne->expr->ext_disp_size();
        ne->ext.resize( s );
        for( int e = 0; e < s; ++e ) {
            CC_SeqItemBlock *b = new CC_SeqItemBlock;
            b->parent_block = ne->parent_block;
            ne->ext[ e ] = b;

            scheduling( b, ne->expr->ext[ e ] );
        }

        // add internal break or continue if necessary
        CC_SeqItemBlock *b[ s ];
        for( int i = 0; i < s; ++i )
            b[ i ] = ne->ext[ i ].ptr();
        ne->expr->add_break_and_continue_internal( b );
    }


}

//void simplifications( Vec<Expr> &created, Vec<Expr> &out ) {
//    for( Expr &e : created ) {
//        if ( e->op_num() == ID_OP_not_boolean ) {
//            if ( e->inp[ 0 ]->op_num() == ID_OP_inf ) {

//            }
//        }
//    }
//}

void Codegen_C::make_code() {
    // a clone of the whole hierarchy
    ++Inst::cur_op_id;
    Vec<Expr> out, created;
    for( Expr inst : fresh )
        out << cloned( inst, created );

    // simplifications
    //    simplifications( created, out );

    // display
    if ( disp_inst_graph )
        Inst::display_graph( out );

    // replication of inlined instructions

    // scheduling (-> list of SeqItem)
    CC_SeqItemBlock main_block;
    scheduling( &main_block, out );

    // get reg for each output
    CppRegConstraint reg_constraints;
    main_block.get_constraints( reg_constraints );
    main_block.assign_reg( this, reg_constraints );

    // specify where the registers have to be declared
    for( int n = 0; n < out_regs.size(); ++n ) {
        CppOutReg &reg = out_regs[ n ];
        CC_SeqItemBlock *anc = reg.common_provenance_ancestor();
        anc->reg_to_decl << &reg;
    }

    // write the code
    main_block.write( this );
}



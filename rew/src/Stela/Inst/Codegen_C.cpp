#include "../System/RaiiSave.h"
#include "InstInfo_C.h"
#include "Codegen_C.h"
#include "BoolOpSeq.h"
#include "SelectDep.h"
#include <fstream>
#include "Store.h"
#include "ValAt.h"
#include "Save.h"
#include "Room.h"
#include <limits>
#include "Ip.h"

Codegen_C::Codegen_C() : on( &main_os ), os( &main_os ) {
    nb_regs = 0;
}

void Codegen_C::write_to( Stream &out ) {
    on.nsp = 4;
    make_code();

    // types
    std::map<Type *,Vec<OutReg *> > orbt;
    for( int i = 0; i < out_regs.size(); ++i )
        orbt[ out_regs[ i ].type ] << &out_regs[ i ];

    for( auto iter : orbt )
        iter.first->write_C_decl( out );


    //
    out << "int main() {\n";

    // out_regs
    for( auto iter : orbt ) {
        out << "    " << *iter.first << " ";
        for( int n = 0; n < iter.second.size(); ++n ) {
            if ( n )
                out << ", ";
            out << *iter.second[ n ];
        }
        out << ";\n";
    }

    out << main_os.str();
    out << "}\n";
}

void Codegen_C::exec() {
    TODO;
}

OutReg *Codegen_C::new_out_reg( Type *type ) {
    return out_regs.push_back( type, nb_regs++ );
}

int Codegen_C::new_num_reg() {
    return nb_regs++;
}

void Codegen_C::C_Code::write_to_stream( Stream &os ) const {
    inst->write_to( cc, prec, IIC( inst )->out_reg );
}

Codegen_C::C_Code Codegen_C::code( Expr inst, int prec ) {
    return C_Code{ this, inst, prec };
}


static Expr cloned( const Expr &val, Vec<Expr> &created ) {
    val->clone( created );
    return reinterpret_cast<Inst *>( val->op_mp );
}

void get_front( Vec<Expr> &front, Expr inst ) {
    if ( inst->op_id == inst->cur_op_id )
        return;
    inst->op_id = inst->cur_op_id;

    if ( inst->inp.size() or inst->dep.size() ) {
        for( Expr ch : inst->inp )
            get_front( front, ch );
        for( Expr ch : inst->dep )
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
        if ( ch->op_id < Inst::cur_op_id )
            return false;
    for( const Expr &ch : inst->dep )
        if ( ch->op_id < Inst::cur_op_id )
            return false;

    // ok
    return true;
}


struct AddToNeeded : Inst::Visitor {
    AddToNeeded( Vec<Expr> &needed ) : needed( needed ) { }
    virtual void operator()( Expr expr ) { needed << expr; }
    Vec<Expr> &needed;
};

static void select_to_select_dep( Expr inst ) {
    if ( inst->op_id == inst->cur_op_id )
        return;
    inst->op_id = inst->cur_op_id;

    if ( inst->is_a_Select() ) {
        Expr r = room( inst->size() );
        Expr v = val_at( r, inst->size() );
        Expr store_ok = store( r, inst->inp[ 1 ] );
        Expr store_ko = store( r, inst->inp[ 2 ] );
        Expr n = select_dep( inst->inp[ 0 ], store_ok, store_ko );
        for( Inst::Parent &p : inst->par ) {
            if ( p.ninp >= 0 )
                p.inst->mod_inp( v, p.ninp );
            else
                ERROR( "dep on a Select... weird !" );
            p.inst->add_dep( n );
        }
        inst->clear_children();
        return select_to_select_dep( n );
    }

    for( Expr ch : inst->inp )
        select_to_select_dep( ch );
    for( Expr ch : inst->dep )
        select_to_select_dep( ch );
}

//static int display_graphviz( SplittedVec<CInstBlock,8> &blocks, const char *filename = ".blk.dot" ) {
//    std::ofstream f( filename );
//    f << "digraph Instruction {\n";
//    f << "  node [shape = record];\n";

//    for( int i = 0; i < blocks.size(); ++i ) {
//        f << "  subgraph cluster" << &blocks[ i ] <<" {\n  color=yellow;\n  style=dotted;\n";
//        for( const Expr &inst : blocks[ i ].inst ) {
//            f << "  node" << inst.ptr() << " [label=\"";
//            inst->write_dot( f );
//            f << "\"];\n";
//            for( const Expr &ch : inst->inp )
//                f << "  node" << inst.ptr() << " -> node" << ch.ptr() << ";\n";
//            for( const Expr &ch : inst->dep )
//                f << "  node" << inst.ptr() << " -> node" << ch.ptr() << " [style=dotted];\n";
//        }
//        f << "  label = \"" << blocks[ i ].cond << "\"\n";
//        f << "  }\n";
//        for( const auto &v : blocks[ i ].cond.or_seq )
//            for( const auto &item : v )
//                f << "  node" << blocks[ i ].inst[ 0 ].ptr() <<"  -> node" << item.expr.ptr() << " [ltail=cluster" << &blocks[ i ] << " color=gray];\n";
//    }

//    f << "}";
//    f.close();

//    return system( ( "dot -Tps " + std::string( filename ) + " > " + std::string( filename ) + ".eps && gv " + std::string( filename ) + ".eps" ).c_str() );
//}

static BoolOpSeq anded( const Vec<BoolOpSeq> &cond_stack ) {
    BoolOpSeq res;
    for( const BoolOpSeq &b : cond_stack )
        res = res && b;
    return res;
}

void Codegen_C::make_code() {
    // clone (-> out)
    ++Inst::cur_op_id;
    Vec<Expr> out, created;
    for( Expr inst : fresh )
        out << cloned( inst, created );
    auto cf = raii_save( ip->cst_false );
    auto ct = raii_save( ip->cst_true  );
    ip->cst_false = cloned( ip->cst_false, created );
    ip->cst_true  = cloned( ip->cst_true , created );

    // select -> select_dep
    //    ++Inst::cur_op_id;
    //    for( Expr inst : out )
    //        select_to_select_dep( inst );

    // add "save" children to Select inst
    for( Expr inst : created )
        if ( inst->is_a_Select() )
            for( int i = 1; i < 3; ++i )
                inst->mod_inp( save( inst->inp[ i ] ), i );


    // update inst->when
    for( Expr inst : out )
        inst->update_when( BoolOpSeq( true ) );

    Inst::display_graph( out );

    // get needed expressions
    Vec<Expr> needed;
    ++Inst::cur_op_id;
    AddToNeeded add_to_needed( needed );
    for( Expr inst : out )
        inst->visit( add_to_needed );

    // op_mp = inst_info on all needed expressions
    SplittedVec<InstInfo_C,16> inst_info_list;
    for( Expr inst : needed )
        inst->op_mp = inst_info_list.push_back();

    // update IIC(  )->out_type
    for( Expr inst : needed )
        inst->update_out_type();
    for( Expr inst : needed )
        if ( not IIC( inst )->out_type )
            inst->out_type_proposition( ip->artificial_type_for_size( inst->size() ) );

    // update IIC(  )->out_reg
    for( Expr inst : needed ) {
        if ( inst->is_a_Select() ) {
            OutReg *reg = new_out_reg( IIC( inst )->out_type );
            IIC( inst )->out_reg = reg;
            IIC( inst->inp[ 1 ] )->out_reg = reg;
            IIC( inst->inp[ 2 ] )->out_reg = reg;
        }
    }

    // first basic scheduling to get Block data
    ++Inst::cur_op_id;
    Vec<Expr> front;
    for( Expr inst : out )
        get_front( front, inst );
    ++Inst::cur_op_id;
    for( Expr inst : front )
        inst->op_id = Inst::cur_op_id;

    Vec<Expr> seq;
    ++Inst::cur_op_id;
    Vec<BoolOpSeq> cond_stack;
    cond_stack << true;
    while ( front.size() ) {
        Expr inst;
        // try to find an instruction with the same condition set or an inst the is not going to write anything
        BoolOpSeq cur_cond = anded( cond_stack );
        for( int i = 0; i < front.size(); ++i ) {
            if ( *front[ i ]->when == cur_cond or not front[ i ]->going_to_write_c_code()  ) {
                inst = front[ i ];
                front.remove_unordered( i );
                break;
            }
        }
        // try to find an instruction with only with additional conditions
        if ( not inst ) {
            int best_score = std::numeric_limits<int>::max(), best_index = -1;
            const int score_close = 10000;
            // std::cout << front << std::endl;
            Vec<BoolOpSeq> best_cond_stack;
            for( int i = 0; i < front.size(); ++i ) {
                BoolOpSeq &cond = *front[ i ]->when;
                Vec<BoolOpSeq> trial_cond_stack = cond_stack;
                int score = 0;

                BoolOpSeq poped;
                if ( cond.always( true ) ) {
                    score += trial_cond_stack.size() * score_close;
                    trial_cond_stack.resize( 0 );
                } else {
                    while ( not cond.imply( anded( trial_cond_stack ) ) ) {
                        poped = trial_cond_stack.pop_back_val();
                        if ( poped.or_seq.size() )
                            score += score_close;
                    }
                }
                BoolOpSeq nc = cond - anded( trial_cond_stack );
                trial_cond_stack << nc;

                if ( nc.or_seq.size() ) {
                    if ( nc.imply( not poped ) )
                        score += 1 + 2 * ( nc - not poped ).nb_sub_conds();
                    else
                        score += 2 * nc.nb_sub_conds();
                }

                // std::cout << "  " << cond << " -> " << score << " " << nc << std::endl;
                if ( best_score > score ) {
                    best_cond_stack = trial_cond_stack;
                    best_score = score;
                    best_index = i;
                }
            }
            if ( best_index >= 0 ) {
                inst = front[ best_index ];
                cond_stack = best_cond_stack;
                front.remove_unordered( best_index );
            }
        }
        // else, take a random one
        if ( not inst ) {
            inst = front.back();
            front.pop_back();
        }

        inst->op_id = Inst::cur_op_id; // done
        if ( inst->going_to_write_c_code() )
            cur_cond = inst->when;
        seq << inst;

        // parents
        for( Inst::Parent &p : inst->par ) {
            if ( ready_to_be_scheduled( p.inst ) ) {
                p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                front << p.inst;
            }
        }
    }

    cond_stack.resize( 0 );
    cond_stack << true;
    for( Expr inst : seq ) {
        if ( not inst->going_to_write_c_code() )
            continue;
        if ( *inst->when != anded( cond_stack ) ) {
            BoolOpSeq poped;
            if ( inst->when->always( true ) ) {
                for( int i = cond_stack.size() - 1; i >= 0; --i ) {
                    if ( cond_stack[ i ].or_seq.size() ) {
                        on.nsp -= 4;
                        on << "}";
                    }
                }
                cond_stack.resize( 1 );
            } else {
                while ( not inst->when->imply( anded( cond_stack ) ) ) {
                    poped = cond_stack.pop_back_val();
                    if ( poped.or_seq.size() ) {
                        on.nsp -= 4;
                        on << "}";
                    }
                }
            }
            BoolOpSeq nc = *inst->when - anded( cond_stack );

            if ( nc.or_seq.size() ) {
                if ( nc.imply( not poped ) ) {
                    cond_stack << not poped;
                    nc = nc - not poped;
                    on << "else {";
                    on.nsp += 4;
                }

                if ( nc.or_seq.size() ) {
                    cond_stack << nc;
                    nc.write_to_stream( on.write_beg() << "if ( " );
                    on.write_end( " ) {" );
                    on.nsp += 4;
                }
            }
        }
        inst->write_to( this );
    }
    for( int i = cond_stack.size() - 1; i >= 0; --i ) {
        if ( cond_stack[ i ].or_seq.size() ) {
            on.nsp -= 4;
            on << "}";
        }
    }
}



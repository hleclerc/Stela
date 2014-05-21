#include "../System/RaiiSave.h"
#include "InstInfo_C.h"
#include "Codegen_C.h"
#include "BoolOpSeq.h"
#include "SelectDep.h"
#include "Store.h"
#include "ValAt.h"
#include "Room.h"
#include "Ip.h"

Codegen_C::Codegen_C() : on( &main_os ), os( &main_os ) {
    nb_regs = 0;
}

void Codegen_C::write_to( Stream &out ) {
    on.nsp = 4;
    make_code();

    out << "int main() {\n";
    out << main_os.str();
    out << "}\n";
}

void Codegen_C::exec() {
    TODO;
}

int Codegen_C::new_num_reg() {
    return nb_regs++;
}

void Codegen_C::C_Code::write_to_stream( Stream &os ) const {
    inst->write_to( cc, prec, IIC( inst )->num_reg );
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


struct CInstBlock {
    CInstBlock( BoolOpSeq cond ) : cond( cond ), op_id( 0 ) {}

    void update_dep() {
        dep.resize( 0 );
        op_id = ++cur_op_id;
        for( Expr i : inst ) {
            for( Expr ch : i->inp ) {
                CInstBlock *b = IIC( ch )->block;
                if ( b->op_id != cur_op_id ) {
                    b->op_id = cur_op_id;
                    b->par << this;
                    dep << b;
                }
            }
            for( Expr ch : i->dep ) {
                CInstBlock *b = IIC( ch )->block;
                if ( b->op_id != cur_op_id ) {
                    b->op_id = cur_op_id;
                    b->par << this;
                    dep << b;
                }
            }
        }
    }

    Vec<CInstBlock *> dep, par;
    Vec<Expr> inst;
    BoolOpSeq cond; ///< cond as a vec of anded op

    static  PI64 cur_op_id;
    mutable PI64 op_id;
};

PI64 CInstBlock::cur_op_id = 0;

bool ready_to_be_scheduled( const CInstBlock *b ) {
    // already in the front ?
    if ( b->op_id >= b->cur_op_id - 1 )
        return false;
    // not computed ?
    for( const CInstBlock *ch : b->dep )
        if ( ch->op_id < b->cur_op_id )
            return false;
    // ok
    return true;
}

static bool ready_to_be_scheduled( Expr inst, CInstBlock *b ) {
    // already in the front ?
    if ( inst->op_id >= Inst::cur_op_id - 1 )
        return false;
    // not computed ?
    for( const Expr &ch : inst->inp )
        if ( ch->op_id < Inst::cur_op_id and IIC( ch )->block == b )
            return false;
    for( const Expr &ch : inst->dep )
        if ( ch->op_id < Inst::cur_op_id and IIC( ch )->block == b )
            return false;
    // ok
    return true;
}

struct CBlockAsm {
    CBlockAsm( CBlockAsm *par = 0, const BoolOpSeq &cond = BoolOpSeq( true ) ) : par( par ), cond( cond ) {
    }
    struct Item {
        CInstBlock *b;
        CBlockAsm  *s;
    };
    CBlockAsm *par;
    BoolOpSeq  cond; ///< cond as a vec of anded op
    Vec<Item>  items;
};

struct AddToNeeded : Inst::Visitor {
    AddToNeeded( Vec<Expr> &needed ) : needed( needed ) {
    }
    virtual void operator()( Expr expr ) {
        needed << expr;
    }
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
    ++Inst::cur_op_id;
    for( Expr inst : out )
        select_to_select_dep( inst );

    // update inst->when
    // pour éviter les boucles: on fait une liste d'exploration...
    for( Expr inst : out )
        inst->update_when( BoolOpSeq( true ) );

    Inst::display_graph( out );

    // the place to make simplifications

    // get needed expressions
    Vec<Expr> needed;
    ++Inst::cur_op_id;
    AddToNeeded add_to_needed( needed );
    for( Expr inst : out )
        inst->visit( add_to_needed );
    PRINT( needed );

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

    //    for( Expr inst : needed )
    //        if ( IIC( inst )->out_type )
    //            std::cout << *IIC( inst )->out_type << "\t" << inst << std::endl;
    //        else
    //            std::cout << "???\t" << inst << std::endl;

    // basic scheduling to get Block data
    ++Inst::cur_op_id;
    Vec<Expr> front;
    for( Expr inst : out )
        get_front( front, inst );
    ++Inst::cur_op_id;
    for( Expr inst : front )
        inst->op_id = Inst::cur_op_id;

    ++Inst::cur_op_id;
    SplittedVec<CInstBlock,8> blocks;
    CInstBlock *cur_block = blocks.push_back( BoolOpSeq( true ) );
    while ( front.size() ) {
        Expr inst;
        for( int i = 0; i < front.size(); ++i ) {
            if ( *front[ i ]->when == cur_block->cond ) {
                inst = front[ i ];
                front.remove_unordered( i );
                break;
            }
        }
        if ( not inst ) {
            if ( not front.size() )
                ERROR( "??" );
            inst = front.back();
            front.pop_back();

            cur_block = blocks.push_back( *inst->when );
        }

        inst->op_id = Inst::cur_op_id; // done
        IIC( inst )->block = cur_block;
        cur_block->inst << inst;

        // parents
        for( Inst::Parent &p : inst->par ) {
            if ( ready_to_be_scheduled( p.inst ) ) {
                p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                front << p.inst;
            }
        }
    }

    // block scheduling
    Vec<CInstBlock *> front_block;
    for( int num_block = 0; num_block < blocks.size(); ++num_block ) {
        CInstBlock *b = &blocks[ num_block ];
        b->update_dep();
        if ( not b->dep.size() )
            front_block << b;
    }

    ++CInstBlock::cur_op_id;
    for( CInstBlock *b : front_block )
        b->op_id = b->cur_op_id;

    //    for( int num_block = 0; num_block < blocks.size(); ++num_block ) {
    //        CInstBlock *b = &blocks[ num_block ];
    //        std::cout << b << std::endl;
    //        std::cout << "  par=" << b->par << std::endl;
    //        std::cout << "  dep=" << b->dep << std::endl;
    //        std::cout << "  cond=" << b->cond << std::endl;
    //        std::cout << "  inst=" << b->inst << std::endl;
    //    }

    ++CInstBlock::cur_op_id;
    SplittedVec<CBlockAsm,8> block_asm;
    CBlockAsm *cba = block_asm.push_back();
    // cba->sc << std::make_pair( ip->cst_true, 1 );
    while ( front_block.size() ) {
        CInstBlock *b = 0;
        // try to find a block with
        // - the same set if conditions
        // - or few additionnal conditions
        // - or few conditions to remove
        // Rq: if ( b and a ) ... if ( a ) ... -> if ( a ) { if ( b ) ... ... }
        b = front_block.back();
        front_block.pop_back();

        b->op_id = b->cur_op_id;

        if ( b->cond == cba->cond )
            cba->items << CBlockAsm::Item{ b, 0 };
        else {
            CBlockAsm *nba = block_asm.push_back( cba, b->cond );
            nba->items << CBlockAsm::Item{ b, 0 };

            cba->items << CBlockAsm::Item{ 0, nba };
        }
        //        while ( not cba->cond->checked_if( b->cond ) )
        //            cba = cba->par;

        // parents
        for( CInstBlock *p : b->par ) {
            if ( ready_to_be_scheduled( p ) ) {
                p->op_id = b->cur_op_id - 1; // -> in the front
                front_block << p;
            }
        }
    }

    write( block_asm[ 0 ] );
    // display
    // for( Expr inst : needed )
    //     std::cout << inst << " when " << inst->when << "\n";
}

//static bool same_cond_with_an_else( Vec<std::pair<Expr,bool> > &a, Vec<std::pair<Expr,bool> > &b ) {
//    return a.size() and
//           a.size() == b.size() and
//           a.slice( 0, a.size() - 1 ) == b.slice( 0, a.size() - 1 ) and
//           a.back().first == b.back().first and
//           a.back().second != b.back().second;
//}

void Codegen_C::write( CBlockAsm &cba ) {
    CBlockAsm *prev = 0;
    for( CBlockAsm::Item &item : cba.items ) {
        if ( item.s ) {
            //if ( prev and same_cond_with_an_else( prev->cond, item.s->cond ) ) {
            //    on << "else {";
            //} else {
                on.write_beg();
                *os << "if ( ";
                //                for( int i = 0; i < item.s->cond.size(); ++i ) {
                //                    if ( i )
                //                        *os << " and ";
                //                    if ( not item.s->sc[ i ].second )
                //                        *os << "not ";
                //                    *os << item.s->sc[ i ].first;
                //                }
                TODO;
                on.write_end( " ) {" );
            //}
            on.nsp += 4;

            write( *item.s );

            on.nsp -= 4;
            on << "}";

            //
            prev = item.s;
        } else {
            CInstBlock *b = item.b;
            prev = 0;

            // front
            Vec<Expr> front;
            ++Inst::cur_op_id;
            for( Expr inst : item.b->inst ) {
                int nb_ch = 0;
                for( Expr ch : inst->inp )
                    nb_ch += IIC( ch )->block == b;
                for( Expr ch : inst->dep )
                    nb_ch += IIC( ch )->block == b;
                if ( not nb_ch ) {
                    inst->op_id = inst->cur_op_id;
                    front << inst;
                }
            }

            //
            ++Inst::cur_op_id;
            while ( front.size() ) {
                Expr inst = front.back();
                front.pop_back();

                inst->write_to( this );
                inst->op_id = inst->cur_op_id;

                // parents
                for( Inst::Parent &p : inst->par ) {
                    if ( IIC( p.inst )->block == b and ready_to_be_scheduled( p.inst, b ) ) {
                        p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                        front << p.inst;
                    }
                }
            }
        }
    }
}


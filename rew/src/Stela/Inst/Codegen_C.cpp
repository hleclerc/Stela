#include "../System/RaiiSave.h"
#include "InstInfo_C.h"
#include "Codegen_C.h"
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
    CInstBlock( Expr cond ) : cond( cond ), op_id( 0 ) {}

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

    void update_sc() {
        cond->_get_sub_cond_and( sc, true );
    }

    Vec<std::pair<Expr,bool> > sc; ///< cond as a vec of anded op
    Vec<CInstBlock *> dep, par;
    Vec<Expr> inst;
    Expr cond;

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


struct CBlockAsm {
    CBlockAsm( CBlockAsm *par = 0, const Vec<std::pair<Expr,bool> > &sc = Vec<std::pair<Expr,bool> >() ) : sc( sc ), par( par ) {
    }
    struct Item {
        CInstBlock *b;
        CBlockAsm  *s;
    };
    Vec<std::pair<Expr,bool> > sc; ///< cond as a vec of anded op
    CBlockAsm                 *par;
    Vec<Item>                  items;
};

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

    // op_mp = inst_info
    SplittedVec<InstInfo_C,16> inst_info_list;
    for( Expr inst : created )
        inst->op_mp = inst_info_list.push_back( ip->cst_false );

    // update IIC(  )->when
    for( Expr inst : out )
        inst->_update_when_C( ip->cst_true );

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
    CInstBlock *cur_block = blocks.push_back( ip->cst_true );
    while ( front.size() ) {
        Expr inst;
        for( int i = 0; i < front.size(); ++i ) {
            if ( IIC( front[ i ] )->when == cur_block->cond ) {
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

            cur_block = blocks.push_back( IIC( inst )->when );
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
        b->update_sc();
        b->update_dep();
        if ( not b->dep.size() )
            front_block << b;
    }

    ++CInstBlock::cur_op_id;
    for( CInstBlock *b : front_block )
        b->op_id = b->cur_op_id;

    for( int num_block = 0; num_block < blocks.size(); ++num_block ) {
        CInstBlock *b = &blocks[ num_block ];
        std::cout << b << std::endl;
        std::cout << "  par=" << b->par << std::endl;
        std::cout << "  dep=" << b->dep << std::endl;
        std::cout << "  cond=" << b->cond << std::endl;
        std::cout << "  inst=" << b->inst << std::endl;
    }

    ++CInstBlock::cur_op_id;
    SplittedVec<CBlockAsm,8> block_asm;
    CBlockAsm *cba = block_asm.push_back();
    cba->sc << std::make_pair( ip->cst_true, 1 );
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

        if ( b->sc == cba->sc )
            cba->items << CBlockAsm::Item{ b, 0 };
        else {
            CBlockAsm *nba = block_asm.push_back( cba, b->sc );
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
    //for( Expr inst : created )
    //    std::cout << inst << " when " << IIC( inst )->when << "\n";

    // Inst::display_graph( out );
}

void Codegen_C::write( CBlockAsm &cba ) {
    for( CBlockAsm::Item &item : cba.items ) {
        if ( item.s ) {
            on.write_beg();
            *os << "if ( ";
            for( int i = 0; i < item.s->sc.size(); ++i ) {
                if ( i )
                    *os << " and ";
                if ( not item.s->sc[ i ].second )
                    *os << " not ";
                *os << item.s->sc[ i ].first;
            }
            on.write_end( " ) {" );
            on.nsp += 4;

            write( *item.s );

            on.nsp -= 4;
            on << "}";
        } else {
            for( Expr inst : item.b->inst )
                on << inst;
        }
    }
}


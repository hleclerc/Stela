#include "../System/SplittedVec.h"
#include "../Inst/Type.h"
#include "../Inst/Cst.h"
#include "InstInfo_C.h"
#include "CodeGen_C.h"
#include <limits>
#include <deque>
#include <map>

CodeGen_C::CodeGen_C() : on( &main_os ), os( &main_os ) {
    nb_regs = 0;
}

void CodeGen_C::write_to( Stream &out ) {
    on.nsp = 4;
    make_code();

    out << "int main() {\n";
    out << main_os.str();
    out << "}\n";
}

void CodeGen_C::exec() {

}

static void get_front_rec( Ptr<Inst> inst, Vec<Ptr<Inst> > &front ) {
    if ( inst->op_id == Inst::cur_op_id )
        return;
    inst->op_id = Inst::cur_op_id;

    if ( inst->inp.size() or inst->dep.size() ) {
        for( Ptr<Inst> ch : inst->inp )
            get_front_rec( ch, front );
        for( Ptr<Inst> ch : inst->dep )
            get_front_rec( ch, front );
    } else
        front << inst;
}

static bool has_ninp_parent( const Inst *inst ) {
    for( const Inst::Parent &p : inst->par )
        if ( p.ninp >= 0 )
            return true;
    return false;
}

// check if all the children have already been scheduled
static bool ready_to_be_scheduled( Ptr<Inst> inst ) {
    // already in the front ?
    if ( inst->op_id >= Inst::cur_op_id - 1 )
        return false;

    // not computed ?
    for( const Ptr<Inst> &ch : inst->inp )
        if ( ch->op_id < Inst::cur_op_id )
            return false;
    for( const Ptr<Inst> &ch : inst->dep )
        if ( ch->op_id < Inst::cur_op_id )
            return false;

    // ok
    return true;
}

static Ptr<Inst> cloned( const Ptr<Inst> &val, Vec<Ptr<Inst> > &created ) {
    val->clone( created );
    return reinterpret_cast<Inst *>( val->op_mp );
}

struct MakeInstBlock : Inst::Visitor {
    void operator()( Inst *inst ) {
        blocks[ IIC( inst )->when ] << inst;
    }
    std::map<Ptr<Inst>,Vec<Ptr<Inst> > > blocks;
};

struct InstBlock {
    InstBlock() : op_id( 0 ) {
    }
    void update_deps() {
        for( Ptr<Inst> i : inst ) {
            for( Ptr<Inst> ch : i->inp )
                if ( IIC( ch )->block != this )
                    dep.push_back_unique( IIC( ch )->block );
            for( Ptr<Inst> ch : i->dep )
                if ( IIC( ch )->block != this )
                    dep.push_back_unique( IIC( ch )->block );
        }
        for( InstBlock *ch : dep )
            ch->par << this;
    }
    void reset_dep() {
        dep.resize( 0 );
        par.resize( 0 );
    }
    bool dep_rec_on( const InstBlock *b ) const {
        ++cur_op_id;
        return _dep_rec_on( b );
    }
    bool _dep_rec_on( const InstBlock *b ) const {
        if ( op_id != cur_op_id ) {
            op_id = cur_op_id;
            for( const InstBlock *c : dep )
                if ( c == b or c->_dep_rec_on( b ) )
                    return true;
        }
        return false;
    }

    Ptr<Inst> cond;
    Vec<Ptr<Inst> > inst;
    Vec<InstBlock *> dep; ///< dependencies
    Vec<InstBlock *> par; ///< parents
    mutable PI64 op_id;
    static  PI64 cur_op_id;
};

PI64 InstBlock::cur_op_id = 0;

static bool ready_to_be_scheduled( InstBlock *ib ) {
    // already in the front ?
    if ( ib->op_id >= InstBlock::cur_op_id - 1 )
        return false;

    // not computed ?
    for( const InstBlock *ch : ib->dep )
        if ( ch->op_id < InstBlock::cur_op_id )
            return false;

    // ok
    return true;
}


void CodeGen_C::make_code() {
    // clone -> out
    ++Inst::cur_op_id;
    Vec<Ptr<Inst> > out, created;
    for( ConstPtr<Inst> inst : fresh ) {
        inst->clone( created );
        out << reinterpret_cast<Inst *>( inst->op_mp );
    }
    Ptr<Inst> inst_false = cloned( cst( false ), created );
    Ptr<Inst> inst_true  = cloned( cst( true  ), created );

    // inst info
    SplittedVec<InstInfo_C,16> inst_info_list;
    for( Ptr<Inst> inst : created )
        inst->op_mp = inst_info_list.push_back( inst_false );

    // set IIC( inst )->when attributes
    for( Ptr<Inst> ch : out )
        ch->add_when_cond( inst_true );

    // make inst_blocks
    MakeInstBlock mib;
    ++Inst::cur_op_id;
    for( Ptr<Inst> ch : out )
        ch->rec_visit( mib );
    SplittedVec<InstBlock,8> inst_blocks;
    for( auto iter : mib.blocks ) {
        InstBlock *ib = inst_blocks.push_back();
        ib->cond = iter.first;
        ib->inst = iter.second;
        for( Ptr<Inst> inst : iter.second )
            IIC( inst )->block = ib;
    }

    // get the block dependencies
    std::deque<InstBlock *> to_split;
    for( int i = 0; i < inst_blocks.size(); ++i ) {
        inst_blocks[ i ].update_deps();
        to_split.push_back( &inst_blocks[ i ] );
    }

    // split the blocks
    while ( to_split.size() ) {
        InstBlock *ib = to_split.front();
        to_split.pop_front();

        // ib does not need to be splitted ?
        if ( not ib->dep_rec_on( ib ) )
            continue;

        // leaves (instruction that does not depend on a block depending of ib)
        Vec<Ptr<Inst> > front;
        Ptr<Inst> cond = ib->cond;
        for( Ptr<Inst> inst : ib->inst ) {
            // nb children depending on a block depending of ib
            int n = 0;
            for( Ptr<Inst> ch : inst->inp )
                n += IIC( ch )->block->dep_rec_on( ib );
            for( Ptr<Inst> ch : inst->dep )
                n += IIC( ch )->block->dep_rec_on( ib );
            if ( not n )
                front << inst;
        }
        if ( not front.size() ) {
            to_split.push_back( ib ); // respawn
            continue;
        }

        // do what can be done without inst from blocks that depend on ib
        Inst::cur_op_id += 2;
        for( Ptr<Inst> ch : front )
            ch->op_id = Inst::cur_op_id - 1; // ch is in the front
        for( InstBlock *db : ib->dep )
            if ( not db->dep_rec_on( ib ) )
                for( const Ptr<Inst> &ch : db->inst )
                    ch->op_id = Inst::cur_op_id; // inst on blocks that do not depend on ib are done
        int nb_inst_in_block = 0;
        while ( front.size() ) {
            Ptr<Inst> inst = front.back();
            front.pop_back();

            inst->op_id = Inst::cur_op_id; // done
            ++nb_inst_in_block;

            for( Inst::Parent &p : inst->par ) {
                if ( IIC( p.inst )->block == ib and ready_to_be_scheduled( p.inst ) ) {
                    p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                    front << p.inst;
                }
            }
        }


        // make a new block with not sweeped instruction.
        ASSERT( nb_inst_in_block != ib->inst.size(), "weird" );
        InstBlock *nb = inst_blocks.push_back();
        to_split.push_back( nb );
        nb->cond = ib->cond;
        Vec<Ptr<Inst> > old_vec = ib->inst;
        ib->inst.resize( 0 );
        for( Ptr<Inst> inst : old_vec ) {
            if ( inst->op_id == Inst::cur_op_id )
                ib->inst << inst;
            else {
                nb->inst << inst;
                IIC( inst )->block = nb;
            }
        }

        // update dep (can be optimized with block->parents)
        for( int i = 0; i < inst_blocks.size(); ++i )
            inst_blocks[ i ].reset_dep();
        for( int i = 0; i < inst_blocks.size(); ++i )
            inst_blocks[ i ].update_deps();
    }

    // leaf blocks
    ++InstBlock::cur_op_id;
    Vec<InstBlock *> front_block;
    for( int i = 0; i < inst_blocks.size(); ++i ) {
        InstBlock *ib = &inst_blocks[ i ];
        if ( not ib->dep.size() ) {
            front_block << ib;
            ib->op_id = InstBlock::cur_op_id;
        }
    }

    PRINT( inst_blocks.size() );

    // schedule the blocks
    ++InstBlock::cur_op_id;
    while ( front_block.size() ) {
        InstBlock *ib = front_block.back();
        ib->op_id = InstBlock::cur_op_id;
        front_block.pop_back();

        PRINT( ib->inst );

        // condensation
        for( const Ptr<Inst> &inst : ib->inst )
            for( const Ptr<Inst> &ch : inst->inp )
                if ( ch->par.size() == 1 and ( IIC( ch )->block == ib or ch == ib->cond ) )
                    IIC( ch )->sched_in = inst.ptr();

        // schedule inst in block
        // -> inst leaves
        Vec<Ptr<Inst> > front;
        ++Inst::cur_op_id;
        for( const Ptr<Inst> &inst : ib->inst ) {
            int n = 0;
            for( const Ptr<Inst> &ch : inst->inp )
                n += IIC( ch )->block == ib;
            for( const Ptr<Inst> &ch : inst->dep )
                n += IIC( ch )->block == ib;
            if ( not n ) {
                inst->op_id = Inst::cur_op_id;
                front << inst;
            }
        }

        Vec<Ptr<Inst> > seq;
        ++Inst::cur_op_id;
        while ( front.size() ) {
            Ptr<Inst> inst = front.back();
            front.pop_back();

            seq << inst;
            inst->op_id = Inst::cur_op_id;

            for( Inst::Parent &p : inst->par ) {
                if ( IIC( p.inst )->block == ib ) {
                    if ( ready_to_be_scheduled( p.inst ) ) {
                        p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                        front << p.inst;
                    }
                } else if ( p.ninp >= 0 and ib->cond != inst_true )
                    decl_if_nec( inst.ptr() );
            }
        }

        // output
        if ( ib->cond != inst_true ) {
            on << "if ( " << code( ib->cond ) << " ) {";
            on.nsp += 4;
        }

        for( Ptr<Inst> inst : seq )
            if ( not IIC( inst )->sched_in )
                inst->write_to( this );

        if ( ib->cond != inst_true ) {
            on.nsp -= 4;
            on << "}";
        }

        // next blocks in front
        for( InstBlock *p : ib->par ) {
            if ( ready_to_be_scheduled( p ) ) {
                p->op_id = InstBlock::cur_op_id - 1; // -> in the front
                front_block << p;
            }
        }
    }

    // Inst::display_graph( out );
}

void get_and_lst( const Ptr<Inst> &src, Vec<Ptr<Inst> > &src_lst ) {
    if ( src->is_and() ) {
        get_and_lst( src->inp[ 0 ], src_lst );
        get_and_lst( src->inp[ 1 ], src_lst );
    } else
        src_lst << src;
}

static int cond_dist( const Ptr<Inst> &src, const Ptr<Inst> &dst ) {
    Vec<Ptr<Inst> > src_lst; get_and_lst( src, src_lst );
    Vec<Ptr<Inst> > dst_lst; get_and_lst( dst, dst_lst );
    for( int i = 0, m = std::min( src_lst.size(), dst_lst.size() ); ; ++i )
        if ( i == m or src_lst[ i ] != dst_lst[ i ] )
            return src_lst.size() + dst_lst.size() - 2 * i; // nb blocs to close
    return 0;
}

Ptr<Inst> CodeGen_C::get_next_inst_in_front( Vec<Ptr<Inst> > &front, Ptr<Inst> current_cond ) {
    // inst with when == current_cond ?
    for( int i = 0; i < front.size(); ++i ) {
        Ptr<Inst> prop = front[ i ];
        if ( IIC( prop )->when == current_cond ) {
            front.remove_unordered( i );
            return prop;
        }
    }
    // else, find the min dist
    int min_dist = std::numeric_limits<int>::max(), min_ind;
    for( int i = 0; i < front.size(); ++i ) {
        Ptr<Inst> prop = front[ i ];
        int new_dist = cond_dist( IIC( prop )->when, current_cond );
        if ( min_dist > new_dist ) {
            min_dist = new_dist;
            min_ind = i;
        }
    }
    Ptr<Inst> res = front[ min_ind ];
    front.remove_unordered( min_ind );
    return res;
}


Stream &CodeGen_C::decl_if_nec( const Inst *inst ) {
    if ( IIC( inst )->num_reg < 0 ) {
        IIC( inst )->num_reg = nb_regs++;
        on.write_beg();
        if ( Type *type = inst->out_type_proposition( this ) )
            *os << *type << ' ';
        *os << "R" << IIC( inst )->num_reg;
        on.write_end( ";" );
    }
    return *os;
}

Stream &CodeGen_C::bdef_if_nec( const Inst *inst ) {
    on.write_beg();
    if ( has_ninp_parent( inst ) ) {
        if ( IIC( inst )->num_reg < 0 ) {
            IIC( inst )->num_reg = nb_regs++;
            if ( Type *type = inst->out_type_proposition( this ) )
                *os << *type << ' ';
        }
        *os << "R" << IIC( inst )->num_reg << " = ";
    }
    return *os;
}

void CodeGen_C::C_Code::write_to_stream( Stream &os ) const {
    if ( IIC( inst )->sched_in ) {
        inst->write_1l_to( cc );
    } else
        os << "R" << IIC( inst )->num_reg;
}

CodeGen_C::C_Code CodeGen_C::code( const Ptr<Inst> &inst ) {
    return C_Code{ this, inst.ptr() };
}



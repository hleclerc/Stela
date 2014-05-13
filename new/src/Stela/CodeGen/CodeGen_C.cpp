#include "../System/SplittedVec.h"
#include "../Inst/Type.h"
#include "../Inst/Cst.h"
#include "InstInfo_C.h"
#include "CodeGen_C.h"
#include <limits>
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

void set_sched_in( const Inst *inst ) {
    if ( inst->op_id == Inst::cur_op_id )
        return;
    inst->op_id = Inst::cur_op_id;

    for( Ptr<Inst> ch : inst->inp ) {
        if ( ch->par.size() == 1 )
            IIC( ch )->sched_in = inst;
        set_sched_in( ch.ptr() );
    }
    for( Ptr<Inst> ch : inst->dep )
        set_sched_in( ch.ptr() );
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
    Ptr<Inst> cond;
    Vec<Ptr<Inst> > inst;
    Vec<InstBlock *> dep; ///< dependencies
};

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
    for( int i = 0; i < inst_blocks.size(); ++i ) {
        InstBlock *ib = &inst_blocks[ i ];
        for( Ptr<Inst> inst : ib->inst ) {
            for( Ptr<Inst> ch : inst->inp )
                if ( IIC( ch )->block != ib )
                    ib->dep << IIC( ch )->block;
            for( Ptr<Inst> ch : inst->dep )
                if ( IIC( ch )->block != ib )
                    ib->dep << IIC( ch )->block;
        }
    }

    // split the blocks
    for( int i = 0; i < inst_blocks.size(); ++i ) {
        InstBlock *ib = &inst_blocks[ i ];

        // leaves
        Vec<Ptr<Inst> > front;
        Ptr<Inst> cond = ib->cond;
        for( Ptr<Inst> inst : ib->inst ) {
            // nb children with same cond
            int n = 0;
            for( Ptr<Inst> ch : inst->inp )
                n += IIC( ch )->when == cond;
            for( Ptr<Inst> ch : inst->dep )
                n += IIC( ch )->when == cond;
            if ( not n )
                front << inst;
        }

        // do what can be done without inst from block that depend on ib
    }


    // condensation (must be done before the leaves)
    ++Inst::cur_op_id;
    for( Ptr<Inst> ch : out )
        set_sched_in( ch.ptr() );

    // leaves
    ++Inst::cur_op_id;
    Vec<Ptr<Inst> > front;
    for( Ptr<Inst> ch : out )
        get_front_rec( ch, front );

    Vec<Ptr<Inst> > seq;
    Inst::cur_op_id += 2;
    Ptr<Inst> current_cond = inst_true;
    while ( front.size() ) {
        Ptr<Inst> inst = get_next_inst_in_front( front, current_cond );
        current_cond = IIC( inst )->when;

        inst->op_id = Inst::cur_op_id; // -> done
        seq << inst;

        std::cout << *inst << " if " << IIC( inst )->when << std::endl;

        for( Inst::Parent &p : inst->par ) {
            if ( ready_to_be_scheduled( p.inst ) ) {
                p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                front << p.inst;
            }
        }
    }

    // output
    for( Ptr<Inst> inst : seq )
        if ( not IIC( inst )->sched_in )
            inst->write_to( this );

    //PRINT( seq );
    Inst::display_graph( out );
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
    on.write_beg();
    if ( has_ninp_parent( inst ) ) {
        IIC( inst )->num_reg = nb_regs++;
        if ( Type *type = inst->out_type_proposition( this ) )
            *os << *type << ' ';
        *os << "R" << IIC( inst )->num_reg <<  " = ";
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



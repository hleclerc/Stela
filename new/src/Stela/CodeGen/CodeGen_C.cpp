#include "../System/SplittedVec.h"
#include "../Inst/Type.h"
#include "InstInfo_C.h"
#include "CodeGen_C.h"

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

    if ( inst->inp.size() or inst->dep.size() or inst->cnd.size() ) {
        for( Ptr<Inst> ch : inst->inp )
            get_front_rec( ch, front );
        for( Ptr<Inst> ch : inst->dep )
            get_front_rec( ch, front );
        for( Ptr<Inst> ch : inst->cnd )
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
    for( const Ptr<Inst> &ch : inst->cnd )
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
    for( Ptr<Inst> ch : inst->cnd ) {
        if ( ch->par.size() == 1 )
            IIC( ch )->sched_in = inst;
        set_sched_in( ch.ptr() );
    }
    for( Ptr<Inst> ch : inst->dep )
        set_sched_in( ch.ptr() );
}

void CodeGen_C::make_code() {
    // clone -> out
    ++Inst::cur_op_id;
    Vec<Ptr<Inst> > out, created;
    for( ConstPtr<Inst> inst : fresh ) {
        inst->clone( created );
        out << reinterpret_cast<Inst *>( inst->op_mp );
    }

    // inst info
    SplittedVec<InstInfo_C,16> inst_info_list;
    for( Ptr<Inst> inst : created )
        inst->op_mp = inst_info_list.push_back();

    // first scheduling, to obtain the ifs

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
    while ( front.size() ) {
        Ptr<Inst> inst = front.back();
        front.pop_back();

        inst->op_id = Inst::cur_op_id; // -> done
        seq << inst;

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



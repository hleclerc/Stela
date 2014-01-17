#include "CppInstCompiler.h"
#include "CppCompiler.h"
#include <fstream>

#define INFO( inst ) reinterpret_cast<CppCompiler::Info *>( inst.op_mp )

CppCompiler::CppCompiler() : on( os ) {
    cpp_filename = "out.cpp";
    nb_regs = 0;
    on.nsp = 4;
}

CppCompiler &CppCompiler::operator<<( ConstPtr<Inst> inst ) {
    outputs << inst;
    return *this;
}

static bool all_children_are_done( PI64 op_id, const Inst *inst ) {
    for( int i = 0; i < inst->inp_size(); ++i )
        if ( inst->inp_expr( i ).inst->op_id < op_id )
            return false;
    return true;
}

void CppCompiler::exec() {
    compile();

    std::ofstream fc( cpp_filename.c_str() );

    for( auto f : includes )
        fc << "#include <" << f << ">\n";

    fc << "int main() {\n";
    fc << os.str();
    fc << "}\n";
}

void CppCompiler::compile() {
    // get the leaves
    Vec<const Inst *> front;
    ++Inst::cur_op_id;
    for( int i = 0; i < outputs.size(); ++i )
        get_front_rec( front, outputs[ i ].ptr() );

    // sweep the tree, starting from the leaves
    PI64 op_id = ++Inst::cur_op_id;
    CppInstCompiler cic( this );
    while ( front.size() ) {
        const Inst *inst = front.pop_back();
        inst->op_id = op_id;

        inst->apply( cic );

        for( int nout = 0; nout < inst->out_size(); ++nout )
            for( const auto &p : inst->out_expr( nout ).parents )
                if ( all_children_are_done( op_id, p.inst ) )
                    front.push_back_unique( p.inst );
    }
}

void CppCompiler::get_front_rec( Vec<const Inst *> &front, const Inst *inst ) {
    if ( inst->op_id == Inst::cur_op_id )
        return;
    inst->op_id = Inst::cur_op_id;
    inst->op_mp = info_it.push_back( inst );

    if ( int nch = inst->inp_size() ) {
        for( int i = 0; i < nch; ++i )
            get_front_rec( front, inst->inp_expr( i ).inst.ptr() );
    } else
        front << inst;

}

struct CppInstTypeHint : InstVisitor {
    virtual void operator()( const Inst &inst ) { hint = 0; }
    virtual void operator()( const Syscall &inst ) { hint = bt_SI64; }
    const BaseType *hint;
    int ninp;
};

CppCompiler::Reg CppCompiler::get_reg_for( const Inst &inst, int nout ) {
    CppCompiler::Reg res;
    res.num  = nb_regs++;

    // find an hint to choose the type
    int bs = 0;
    for( const auto &p : inst.out_expr( nout ).parents ) {
        CppInstTypeHint ch; ch.ninp = p.ninp;
        p.inst->apply( ch );
        if ( ch.hint and bs < ch.hint->size_in_bytes() ) {
            bs = ch.hint->size_in_bytes();
            res.type = ch.hint;
            break;
        }
    }
    ASSERT( res.type, "..." );

    // register
    INFO( inst )->set_out_reg( nout, res );
    return res;
}

void CppCompiler::add_include( String name ) {
    includes.insert( name );
}

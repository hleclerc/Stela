#include "CppInstCompiler.h"

#include "../Interpreter/BaseType.h"

#include "../Interpreter/Syscall.h"
#include "../Interpreter/Rand.h"
#include "../Interpreter/Cst.h"

#define INFO( inst ) reinterpret_cast<CppCompiler::Info *>( inst.op_mp )

CppInstCompiler::CppInstCompiler( CppCompiler *cc ) : cc( cc ) {
}

void CppInstCompiler::operator()( const Inst &inst ) {
    std::ostringstream ss;
    std::cerr << inst << std::endl;
    TODO;
}

void CppInstCompiler::operator()( const Syscall &inst ) {
    cc->on.write_beg();
    cc->os << "syscall( ";
    for( int i = 1; i < inst.inp.size(); ++i ) {
        if ( i > 1 )
            cc->os << ", ";
        cc->os << INFO( inst )->get_inp_reg( i );
    }
    cc->os << " )";
    cc->on.write_end();
}

void CppInstCompiler::operator()( const Rand &inst ) {
    cc->add_include( "stdlib.h" );
    Reg reg = cc->get_reg_for( inst, 0 );
    cc->on.write_beg();
    reg.write_decl( cc->os ) << "rand();";
    cc->on.write_end();
}

void CppInstCompiler::operator()( const Cst &inst ) {
    if ( inst.value.size() == 0 )
        return;
    Reg reg = cc->get_reg_for( inst, 0 );
    cc->on.write_beg();
    reg.type->write_to_stream( reg.write_decl( cc->os ), inst.value.ptr() );
    cc->os << ";";
    cc->on.write_end();
}

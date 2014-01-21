#include "../Inst/BaseType.h"
#include "CppInstCompiler.h"

#define INFO( inst ) reinterpret_cast<CppCompiler::Info *>( inst.op_mp )

CppInstCompiler::CppInstCompiler( CppCompiler *cc ) : cc( cc ) {
}

void CppInstCompiler::def( const Inst &inst ) {
    std::ostringstream ss;
    std::cerr << inst << std::endl;
    TODO;
}

#define DECL_OP( OP ) void CppInstCompiler::OP( const Inst &inst ) { TODO; }
#include "../Inst/DeclOp.h"
#undef DECL_OP

void CppInstCompiler::concat( const Inst &inst ) {
    TODO;
}
void CppInstCompiler::syscall( const Inst &inst, int ptr_size ) {
    cc->on.write_beg();
    cc->os << "syscall( ";
    for( int i = 1; i < inst.inp_size(); ++i ) {
        if ( i > 1 )
            cc->os << ", ";
        cc->os << INFO( inst )->get_inp_reg( i );
    }
    cc->os << " )";
    cc->on.write_end();
}
void CppInstCompiler::val_at( const Inst &inst, int size ) {
    TODO;
}
void CppInstCompiler::slice( const Inst &inst, int beg, int end ) {
    TODO;
}
void CppInstCompiler::pointer_on( const Inst &inst, int ptr_size ) {
    TODO;
}
void CppInstCompiler::rand( const Inst &inst, int size ) {
    cc->add_include( "stdlib.h" );
    Reg reg = cc->get_reg_for( inst, 0 );
    cc->on.write_beg();
    reg.write_decl( cc->os ) << "rand();";
    cc->on.write_end();
}
void CppInstCompiler::cst( const Inst &inst, const Vec<PI8> &data ) {
    if ( data.size() == 0 )
        return;
    Reg reg = cc->get_reg_for( inst, 0 );
    cc->on.write_beg();
    reg.type->write_to_stream( reg.write_decl( cc->os ), data.ptr() );
    cc->os << ";";
    cc->on.write_end();
}

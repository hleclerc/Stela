#include "../Inst/BaseType.h"
#include "CppInstCompiler.h"

#define INFO( inst ) reinterpret_cast<CppInstInfo *>( (inst).op_mp )

CppInstCompiler::CppInstCompiler( CppCompiler *cc, bool inline_inst ) : cc( cc ), inline_inst( inline_inst ) {
}

bool CppInstCompiler::decl( const Inst &inst, int nout ) const {
    return inst.out_expr( nout ).parents.size() > 1 and not inline_inst;
}

void CppInstCompiler::def( const Inst &inst ) {
    std::ostringstream ss;
    std::cerr << inst << std::endl;
    TODO;
}

#define DECL_OP( OP ) void CppInstCompiler::OP( const Inst &inst ) { TODO; }
#include "../Inst/DeclOp.h"
#undef DECL_OP

void CppInstCompiler::phi( const Inst &inst ) {
    TODO;
}

void CppInstCompiler::concat( const Inst &inst ) {
    TODO;
}
void CppInstCompiler::syscall( const Inst &inst ) {
    cc->on.write_beg();
    if ( inst.out_expr( 0 ).parents.size() and not inline_inst )
        cc->os << INFO( inst )->decl_writer( cc, 0 );
    cc->os << "syscall( ";
    for( int i = 1; i < inst.inp_size(); ++i ) {
        if ( i > 1 )
            cc->os << ", ";
        cc->os << INFO( *inst.inp_expr( i ).inst )->inst_writer( cc, inst.inp_expr( i ).nout );
    }
    cc->os << " )";
    if ( not inline_inst )
        cc->os << ";";
    cc->on.write_end();
}

void CppInstCompiler::val_at( const Inst &inst, int beg, int end ) {
    TODO;
}

void CppInstCompiler::slice( const Inst &inst, int beg, int end ) {
    TODO;
}

void CppInstCompiler::pointer_on( const Inst &inst ) {
    TODO;
}

void CppInstCompiler::rand( const Inst &inst, int size ) {
    cc->add_include( "stdlib.h" );
    if ( inline_inst )
        cc->os << "rand()";
    else if ( inst.out_expr( 0 ).parents.size() > 1 )
        cc->on << INFO( inst )->decl_writer( cc, 0 ) << "rand();";
}

void CppInstCompiler::conv( const Inst &inst, const BaseType *dst, const BaseType *src ) {
    TODO;
}

void CppInstCompiler::cst( const Inst &inst, const PI8 *value, const PI8 *known, int size_in_bits ) {
    if ( not size_in_bits )
        return;
    if ( decl( inst, 0 ) )
        cc->on.write_beg() << INFO( inst )->decl_writer( cc, 0 );

    const BaseType *bt = INFO( inst )->out[ 0 ].type;
    ASSERT( bt, "bad" );
    bt->write_to_stream( cc->os, value );

    if ( decl( inst, 0 ) )
        cc->on.write_end( ";" );
}

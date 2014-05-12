#include "Stela/CodeGen/CodeGen_C.h"
#include "Stela/Inst/Type.h"
#include "Stela/Inst/Var.h"
#include "Stela/Inst/Ip.h"

void test_ptr() {
    Var b = 10;
    Var p = b.ptr();

    PRINT( b );
    PRINT( p );
    b.reassign( 20 );
    PRINT( b );
    PRINT( p );
    PRINT( p.ptd() );
    p.ptd().reassign( 150 );
    PRINT( p );
    PRINT( simplified( p.ptd().inst ) );
}

int main() {
    static Ip ip_inst;
    ip = &ip_inst;

    //    Var b( 10 );
    //
    //    b = 15;
    //    PRINT( b );
    Vec<Var> inp( Size(), 1 );
    inp[ 0 ] = 16l;
    syscall( inp );

    ip->add_cond( 25 );
    inp[ 0 ] = 17l;
    syscall( inp );
    ip->pop_cond();

    ip->add_cond( 26 );
    inp[ 0 ] = 18l;
    syscall( inp );
    ip->pop_cond();

    CodeGen_C cg;
    cg << simplified( ip->sys_state.inst );
    cg.write_to( std::cout );
}

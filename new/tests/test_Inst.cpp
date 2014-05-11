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
    //    Var b( 10 );
    //    ip->add_cond( 25 );
    //    b = 15;
    //    PRINT( b );
    Vec<Var> inp;
    inp << 16;
    syscall( inp );


    CodeGen_C cg;
    cg << simplified( ip->sys_state.inst );
    cg.write_to( std::cout );
}

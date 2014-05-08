#include "Stela/Inst/Type.h"
#include "Stela/Inst/Var.h"
#include "Stela/Inst/Ip.h"

int main() {
    Var a( &ip->type_SI32 );
    Var b( 10 );
    Var p( Ref(), b.ptr() );

    PRINT( a );
    PRINT( b );
    PRINT( p );
    b = 20;
    PRINT( b );
    PRINT( p );
    PRINT( p.ptd() );
    p.ptd() = 150;
    PRINT( p );
    PRINT( simplified( p.ptd().inst ) );
}

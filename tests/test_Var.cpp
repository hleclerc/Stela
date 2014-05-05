#include "Stela/Ref/Var.h"
#include "Stela/Ref/Ip.h"

/** A tester:
 set_val avec pointeur

*/
int main() {
    Type st;

    Var s( &st, 64 );

    Var a = Var::from_val( 150 );
    // a.set( b )

    PRINT( a );
    PRINT( s );
}

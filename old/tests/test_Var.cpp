#include "Stela/Ref/Var.h"
#include "Stela/Ref/Ip.h"

/** A tester:
  set_val avec pointeur

  Pb; while avec pointeurs
     Sol 1: RefItem_PointerOn
     Rq: on ne pourrait autoriser des sub que sur la même zone
       avec des add(&p,5) qui renvoient un RefPtr( RefSlice( p, 5 ) ) (le RefItem_Add n'existe pas)
*/
int main() {
    Type t( ip->str_cor.num( "my_type" ) );
    //t.add_field( ip->str_cor.num( "a" ), 15 );
    //t.add_field( ip->str_cor.num( "b" ), 16 );
    t.add_room( 96 );

    Var obj( &t );
    slice( obj, &ip->type_SI32,  0 ) = 17;
    slice( obj, &ip->type_SI32, 32 ) = 18;
    PRINT( obj );
    PRINT( obj.expr() );
}

#include <Stela/Inst/Symbol.h>
#include <Stela/Inst/Ip.h>

void test_ptr() {
    Var b( &ip->type_SI32, symbol( "b", 32 ) );
    Var p = ptr( b );
    Var v = at( &ip->type_SI32, p );
    PRINT( b );
    PRINT( p );
    PRINT( v );
}

void test_cond() {
    Var a( &ip->type_SI32, symbol( "a", 32 ) );
    Var b( &ip->type_SI32, symbol( "b", 32 ) );
    Var c( &ip->type_SI32, symbol( "c", 32 ) );
    Var c0( &ip->type_Bool, symbol( "c0", 1 ) );
    Var c1( &ip->type_Bool, symbol( "c1", 1 ) );
    PRINT( a );
    PRINT( b );

    ip->set_cond( c0 );
    *a = b;
    PRINT( a );

    ip->set_cond( and_boolean( c0, c1 ) );
    // *a = c;
    PRINT( a );
    PRINT( *a );
}

int main() {
    Ip ip_inst; ip = &ip_inst;

    test_cond();
}

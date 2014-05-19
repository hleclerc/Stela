#include <Stela/Inst/Symbol.h>
#include <Stela/Inst/Cst.h>
#include <Stela/Inst/Op.h>
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

Expr _and( Expr c0, Expr c1 ) {
    return op( &ip->type_Bool, &ip->type_Bool, c0, &ip->type_Bool, c1, Op_and_boolean() );
}
Expr _or( Expr c0, Expr c1 ) {
    return op( &ip->type_Bool, &ip->type_Bool, c0, &ip->type_Bool, c1, Op_or_boolean() );
}
Expr _not( Expr c0 ) {
    return op( &ip->type_Bool, &ip->type_Bool, c0, Op_not_boolean() );
}

void test_checked_if() {
    Expr c0 = symbol( "c0", 1 );
    Expr c1 = symbol( "c1", 1 );
    PRINT( ip->cst_false->always_checked() );
    PRINT( ip->cst_true ->always_checked() );
    PRINT( c0->bval_if( c0 ) );
    PRINT( _not( c0 )->bval_if( c0 ) );
    PRINT( c0->bval_if( _not( c0 ) ) );
    PRINT( c0->bval_if( c1 ) );
    PRINT( c0->bval_if( _and( c0, c1 ) ) );
    PRINT( c0->bval_if( _or ( c0, c1 ) ) );
    PRINT( _and( c0, c1 )->bval_if( _and( c0, c1 ) ) );
    PRINT( _and( c0, c1 )->bval_if( c0 ) );
    PRINT( _or ( c0, c1 )->bval_if( c0 ) );
}

int main() {
    Ip ip_inst; ip = &ip_inst;

    // test_cond();
    test_checked_if();
}

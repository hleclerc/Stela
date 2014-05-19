#include <Stela/Inst/Codegen_C.h>
#include <Stela/Inst/Symbol.h>
#include <Stela/Inst/Cst.h>
#include <Stela/Inst/Op.h>
#include <Stela/Inst/Ip.h>

void test_ptr() {
    Var b( &ip->type_SI32, symbol( "b", 32 ) );
    Var p = b.ptr();
    Var v = p.at( &ip->type_SI32 );
    PRINT( b );
    PRINT( p );
    PRINT( v );
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
    PRINT( c0->checked_if( c0 ) );
    PRINT( _not( c0 )->checked_if( c0 ) );
    PRINT( c0->checked_if( _not( c0 ) ) );
    PRINT( c0->checked_if( c1 ) );
    PRINT( c0->checked_if( _and( c0, c1 ) ) );
    PRINT( c0->checked_if( _or ( c0, c1 ) ) );
    PRINT( _and( c0, c1 )->checked_if( _and( c0, c1 ) ) );
    PRINT( _and( c0, c1 )->checked_if( c0 ) );
    PRINT( _or ( c0, c1 )->checked_if( c0 ) );
}

#define PVAL( A ) \
    std::cout << "  " << #A << " -> " << (A).get_val() << std::endl


void test_simp_bool() {
    Var c0( &ip->type_Bool, symbol( "c0", 1 ) );
    Var c1( &ip->type_Bool, symbol( "c1", 1 ) );

    PVAL( c0.and_boolean( c0 ) );
    PVAL( c0.and_boolean( c0.not_boolean() ) );
    PVAL( c0.not_boolean().and_boolean( c0 ) );
    PVAL( c0.not_boolean().and_boolean( c0.not_boolean() ) );
    PVAL( c0.and_boolean( c1.and_boolean( c0 ) ) );
    PVAL( c0.and_boolean( c1.and_boolean( c0.not_boolean() ) ) );

    PVAL( c0.or_boolean( c0 ) );
    PVAL( c0.or_boolean( c0.not_boolean() ) );
    PVAL( c0.not_boolean().or_boolean( c0 ) );
    PVAL( c0.not_boolean().or_boolean( c0.not_boolean() ) );
    PVAL( c0.or_boolean( c1.or_boolean( c0 ) ) );
    PVAL( c0.or_boolean( c1.or_boolean( c0.not_boolean() ) ) );
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
    a.set_val( b );
    PRINT( a );

    ip->set_cond( c0.and_boolean( c1 ) );
    // *a = c;
    PRINT( a );
    PRINT( a.get_val() );
}

void test_graph() {
    Var a( &ip->type_SI32, symbol( "a", 32 ) );
    Var b( &ip->type_SI32, symbol( "b", 32 ) );
    Var c( &ip->type_SI32, symbol( "c", 32 ) );
    Var c0( &ip->type_Bool, symbol( "c0", 1 ) );
    Var c1( &ip->type_Bool, symbol( "c1", 1 ) );

    syscall( a.ptr() );
    a.set_val( b );
    syscall( a.ptr() );
    syscall( a.ptr() );

    // ip->set_cond( c0 );
    // syscall( b );

    // ip->pop_cond();

    Vec<Expr> out;
    // out << *a;
    out << ip->sys_state.get_val();
    Inst::display_graph( out );
}

void test_code() {
    Var a( &ip->type_SI32, symbol( "a", 32 ) );
    Var b( &ip->type_SI32, symbol( "b", 32 ) );
    Var c( &ip->type_SI32, symbol( "c", 32 ) );
    Var c0( &ip->type_Bool, symbol( "c0", 1 ) );
    Var c1( &ip->type_Bool, symbol( "c1", 1 ) );

    syscall( a.ptr() );
    a.set_val( b );
    ip->set_cond( c0 );
    syscall( a.ptr() );
    ip->pop_cond();
    syscall( a.ptr() );

    Codegen_C cc;
    cc << ip->sys_state.get_val();
    cc.write_to( std::cout );
}

int main() {
    Ip ip_inst; ip = &ip_inst;

    // test_checked_if();
    // test_cond();
    // test_graph();
    test_simp_bool();
    // test_code();
}

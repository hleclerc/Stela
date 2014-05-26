#include <Stela/Inst/Codegen_C.h>
#include <Stela/Inst/BoolOpSeq.h>
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

#define PVAL( A ) \
    std::cout << "  " << #A << " -> " << (A).get_val() << std::endl


void test_simp_bool() {
    Var c0( &ip->type_Bool, symbol( "c0", 1 ) );
    Var c1( &ip->type_Bool, symbol( "c1", 1 ) );
    Var c2( &ip->type_Bool, symbol( "c2", 1 ) );
    Var cf( &ip->type_Bool, ip->cst_false );
    Var ct( &ip->type_Bool, ip->cst_true  );

    PVAL( not c0 );
    PVAL( c0 or c0 );
    PVAL( c0 or c1 );
    PVAL( c0 or cf );
    PVAL( c0 or ct );
    PVAL( c0 or not c0 );
    PVAL( not ( c0 or c1 ) );
    PVAL( ( c0 and c1 ) or ( not c0 and c1 ) );

    PVAL( not c0 );
    PVAL( c0 and c0 );
    PVAL( c0 and c1 );
    PVAL( c0 and cf );
    PVAL( c0 and ct );
    PVAL( c0 and not c0 );
    PVAL( ( c0 and c1 ) or c0 );
    PVAL( not ( c0 and c1 ) );

    PVAL( ( c0 and c1 ) or ( c0 and c2 ) );
}

void test_bool_op() {
    BoolOpSeq c0( symbol( "c0", 1 ), true );
    BoolOpSeq c1( symbol( "c1", 1 ), true );
    BoolOpSeq c2( symbol( "c2", 1 ), true );

    PRINT( c0 );
    PRINT( ( c0 and c1 ) or ( c0 and c2 ) );
    PRINT( ( ( c0 and c1 ) or ( c0 and c2 ) ) - c0 );
    PRINT( ( ( c0 and c1 ) or ( c0 and c2 ) ) - ( c0 or c1 ) );
    PRINT( ( ( c0 and c1 ) or ( c0 and c2 ) ) - ( c1 or c2 ) );
    PRINT( ( ( c0 or c1 ) == ( c1 or c0 ) ) );

    PRINT( ( c0 and c1 ).imply( c1 ) );
    PRINT( ( c0 and c1 ).imply( c2 ) );
    PRINT( ( c0 and c1 ).imply( c0 or c1 ) );
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

    ip->set_cond( c0 and c1 );
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

    syscall( Vec<Var>( a.ptr(), b.ptr() ) );
    a.set_val( b );
    syscall( Vec<Var>( a.ptr(), b.ptr() ) );
    syscall( Vec<Var>( a.ptr(), b.ptr() ) );

    // ip->set_cond( c0 );
    // syscall( b );

    // ip->pop_cond();

    Vec<Expr> out;
    // out << *a;
    out << ip->sys_state.get_val();
    Inst::display_graph( out );
}

void test_code_syscall() {
    Var a( &ip->type_SI32, symbol( "a", 32 ) );
    Var b( &ip->type_SI32, symbol( "b", 32 ) );
    Var c( &ip->type_SI32, symbol( "c", 32 ) );
    Var c0( &ip->type_Bool, symbol( "c0", 1 ) );
    Var c1( &ip->type_Bool, symbol( "c1", 1 ) );

    syscall( Vec<Var>( a.ptr()/*, b.ptr()*/ ) );
    a.set_val( b );
    ip->set_cond( c0 );
    syscall( Vec<Var>( a.ptr()/*, b.ptr()*/ ) );
    ip->pop_cond();
    syscall( Vec<Var>( a.ptr()/*, b.ptr()*/ ) );

    Codegen_C cc;
    cc << ip->sys_state.get_val();
    cc.write_to( std::cout );
}

void test_code_select() {
    Var a( &ip->type_SI32, symbol( "a", 32 ) );
    Var b( &ip->type_SI32, symbol( "b", 32 ) );
    Var c( &ip->type_SI32, symbol( "c", 32 ) );
    Var c0( &ip->type_Bool, symbol( "c0", 1 ) );
    Var c1( &ip->type_Bool, symbol( "c1", 1 ) );

    ip->set_cond( c0 );
    a.set_val( b );
    ip->pop_cond();

    ip->set_cond( c1 );
    //ip->set_cond( c0.and_boolean( c1 ) );
    a.set_val( c + b );
    ip->pop_cond();

    syscall( a.ptr() );

    Codegen_C cc;
    cc << ip->sys_state.get_val();
    cc.write_to( std::cout );
}

int main() {
    Ip ip_inst; ip = &ip_inst;

    // test_cond();
    // test_graph();
    //    test_simp_bool();
    //    test_bool_op();
    // test_code_syscall();
    test_code_select();
}

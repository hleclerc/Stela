#include <Stela/Inst/BoolOpSeq.h>
#include <Stela/Inst/ReplBits.h>
#include <Stela/Inst/Syscall.h>
#include <Stela/Inst/Symbol.h>
#include <Stela/Inst/Expr.h>
#include <Stela/Inst/Type.h>
#include <Stela/Inst/Room.h>
#include <Stela/Inst/Op.h>
#include <Stela/Inst/Ip.h>

void test_set() {
    Expr sc = symbol( ip->type_Bool, "cond" );
    Expr sym = symbol( ip->type_SI32, "sym" );
    BoolOpSeq cond( sc, true );

    Expr a = room();
    PRINT( a );
    a->set( 20, BoolOpSeq() );
    a->set( 21, cond );
    a->set( 22, cond );
    PRINT( a );
    PRINT( *Expr( 10 )->type()  );
    PRINT( *a->type()  );
    // Inst::display_graph( a );
}

void test_op() {
    Expr sym = symbol( ip->type_SI32, "sym" );

    PRINT( add( Expr( 10 ), 20 ) );
    PRINT( add( sym, 20 ) );
    PRINT( add( sym, 0 ) );
    PRINT( mul( sym, 0 ) );
    PRINT( mul( sym, 1 ) );
}

void test_repl_bits() {
    PRINT( repl_bits( 20, 0, 7 ) );
    PRINT( repl_bits( 20, 16, PI8( 7 ) ) );
}

void test_syscall() {
    Expr a = room();
    a->set( symbol( ip->type_SI32, "sym" ), BoolOpSeq() );
    syscall( a );
    a->set( symbol( ip->type_SI32, "szm" ), BoolOpSeq() );
    syscall( a );
    Inst::display_graph( ip->sys_state );
}

int main() {
    Ip ip_inst;
    ip = &ip_inst;

    test_syscall();
}

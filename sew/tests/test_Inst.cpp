#include <Stela/Inst/BoolOpSeq.h>
#include <Stela/Inst/ReplBits.h>
#include <Stela/Inst/Syscall.h>
#include <Stela/Inst/Symbol.h>
#include <Stela/Inst/Expr.h>
#include <Stela/Inst/Type.h>
#include <Stela/Inst/Room.h>
#include <Stela/Inst/Op.h>
#include <Stela/Inst/Ip.h>

int main() {
    Ip ip_inst;
    ip = &ip_inst;

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

    PRINT( repl_bits( 20, 0, 7 ) );
    PRINT( repl_bits( 20, 16, PI8( 7 ) ) );

    // pb: gestion des set partiel
    // prop 1: de l'ext, on fait set( repl_bits( obj, off, val ) )
    PRINT( add( Expr( 10 ), 20 ) );
    PRINT( add( sym, 20 ) );
    PRINT( add( sym, 0 ) );
    PRINT( mul( sym, 0 ) );
    PRINT( mul( sym, 1 ) );
}

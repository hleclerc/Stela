#include <Stela/Inst/BoolOpSeq.h>
#include <Stela/Inst/Symbol.h>
#include <Stela/Inst/Expr.h>
#include <Stela/Inst/Room.h>
#include <Stela/Inst/Ip.h>

int main() {
    Ip ip_inst;
    ip = &ip_inst;

    BoolOpSeq cond( symbol( ip->type_Bool, "cond" ), true );

    Expr a = room();
    PRINT( a );
    a->set( 20, BoolOpSeq() );
    a->set( 21, cond );
    // a = 10;
    PRINT( a );
}

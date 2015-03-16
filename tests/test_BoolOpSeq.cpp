#include <Stela/Inst/BoolOpSeq.h>
#include <Stela/Inst/Symbol.h>
#include <Stela/Inst/Ip.h>

int main() {
    Ip ip_inst; ip = &ip_inst;
    Expr eA = symbol( ip->type_ST, "A" );
    Expr eB = symbol( ip->type_ST, "B" );
    BoolOpSeq A( eA, true );
    BoolOpSeq B( eB, true );
    PRINT( A );
    PRINT( not A );
    PRINT( ( ( not A ) or A ) );
    PRINT( ( A or ( not A ) ) );
    PRINT( ( A or ( not A ) ) );

    PRINT( ( A and B ) or A );
    PRINT( A or ( A and B ) );
}

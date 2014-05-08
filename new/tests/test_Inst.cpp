#include "Stela/Inst/Type.h"
#include "Stela/Inst/Var.h"
#include "Stela/Inst/Ip.h"

int main() {
    Var a( &ip->type_SI32 );
    Var b( 10 );

    PRINT( a );
    PRINT( b );
    // pb: si on ne stocke qu'un pointeur dans une Var,
    //  l'état de la Var est perdu :(
    //  (sauf si le pointeur stocke une Var)
}

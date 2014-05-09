#include "Stela/Inst/Type.h"
#include "Stela/Inst/Var.h"
#include "Stela/Inst/Ip.h"

void test_ptr() {
    Var b( 10 );
    Var p( Ref(), b.ptr() );

    PRINT( b );
    PRINT( p );
    b = 20;
    PRINT( b );
    PRINT( p );
    PRINT( p.ptd() );
    p.ptd() = 150;
    PRINT( p );
    PRINT( simplified( p.ptd().inst ) );
}

int main() {
    Var b( 10 );
    ip->add_cond( 25 );
    b = 15;
    PRINT( b );

    Vec<Ptr<Inst> > out;
    out << b.inst;
    Inst::display_graph( out );
}

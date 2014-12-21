#include "Codegen.h"
#include "OutReg.h"

Codegen::Codegen() : on( &ms ), os( &ms ) {
    num_reg = 0;
}

Codegen &Codegen::operator<<( Expr inst ) {
    fresh << inst;
    return *this;
}

void Codegen::set_os( Stream *_os, int nsp ) {
    os = _os;
    on.stream = _os;
    if ( nsp >= 0 )
        on.nsp = nsp;
}

OutReg *Codegen::new_reg() {
    return new OutReg( "R" + to_string( num_reg++ ) );
}

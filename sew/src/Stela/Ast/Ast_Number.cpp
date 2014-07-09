#include "Ast_Number.h"

Ast_Number::Ast_Number( int off ) : Ast( off ) {
    l = false;
    p = false;
}

void Ast_Number::write_to_stream( Stream &os, int nsp ) const {
    os << str;
    if ( l ) os << 'l';
    if ( p ) os << 'p';
}


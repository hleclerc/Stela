#include "Ast_Continue.h"

Ast_Continue::Ast_Continue( int off ) : Ast( off ) {
    n = 1;
}

void Ast_Continue::write_to_stream( Stream &os, int nsp ) const {
    os << "Continue";
}


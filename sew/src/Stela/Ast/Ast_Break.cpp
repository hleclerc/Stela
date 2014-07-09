#include "Ast_Break.h"

Ast_Break::Ast_Break( int off ) : Ast( off ) {
    n = 1;
}

void Ast_Break::write_to_stream( Stream &os, int nsp ) const {
    os << "Break[" << n << "]";
}


#include "Ast_For.h"

Ast_For::Ast_For( int off ) : Ast( off ) {
}

void Ast_For::write_to_stream( Stream &os, int nsp ) const {
    os << "For";
}


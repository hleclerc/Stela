#include "Ast_Return.h"

Ast_Return::Ast_Return( int off ) : Ast( off ) {
}

void Ast_Return::write_to_stream( Stream &os, int nsp ) const {
    os << "return ";
    val->write_to_stream( os, nsp + 2 );
}


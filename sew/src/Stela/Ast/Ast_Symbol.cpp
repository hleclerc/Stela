#include "Ast_Symbol.h"

Ast_Symbol::Ast_Symbol( String str ) : str( str ) {
}

void Ast_Symbol::write_to_stream(Stream &os, int nsp) const {
    os << str;
}

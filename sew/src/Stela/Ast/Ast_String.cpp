#include "Ast_String.h"

Ast_String::Ast_String( int off, String str ) : Ast( off ), str( str ) {
}

void Ast_String::write_to_stream( Stream &os, int nsp ) const {
    os << str;
}


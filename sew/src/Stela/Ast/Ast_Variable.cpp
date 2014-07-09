#include "Ast_Variable.h"

Ast_Variable::Ast_Variable( int off, String str ) : Ast( off ), str( str ) {
}

void Ast_Variable::write_to_stream( Stream &os, int nsp ) const {
    os << str;
}

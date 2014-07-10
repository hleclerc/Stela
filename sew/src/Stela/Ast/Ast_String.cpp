#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_String.h"

Ast_String::Ast_String( int off, String str ) : Ast( off ), str( str ) {
}

void Ast_String::write_to_stream( Stream &os, int nsp ) const {
    os << str;
}


void Ast_String::_get_info( IrWriter *aw ) const {
    TODO;
}

PI8 Ast_String::_tok_number() const {
    return IR_TOK_STRING;
}

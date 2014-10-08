#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Number.h"

Ast_Number::Ast_Number( int off ) : Ast( off ) {
    l = false;
    p = false;
    b = false;
}

Ast_Number::Ast_Number( int off, bool val ) : Ast_Number( off ) {
    str = val ? "1" : "0";
    b = true;
}

Ast_Number::Ast_Number( int off, String str ) : Ast_Number( off ) {
    this->str = str;
}

void Ast_Number::write_to_stream( Stream &os, int nsp ) const {
    os << str;
    if ( l ) os << 'l';
    if ( p ) os << 'p';
}


void Ast_Number::_get_info( IrWriter *aw ) const {
    if ( b )
        return;
    aw->data << atoi( str.c_str() );
}

PI8 Ast_Number::_tok_number() const {
    if ( b )
        return str == "1" ? IR_TOK_TRUE : IR_TOK_FALSE;
    if ( p )
        return IR_TOK_PTR;
    if ( l )
        return IR_TOK_SI64;
    return IR_TOK_SI32;
}

Ast::Past Ast_Number::_parse_in( ConvScope &scope ) {
    return this;
}

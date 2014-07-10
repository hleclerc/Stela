#include "../Ir/Numbers.h"
#include "IrWriter.h"
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


void Ast_Number::_get_info( IrWriter *aw ) const {
    aw->data << atoi( str.c_str() );
}

PI8 Ast_Number::_tok_number() const {
    return IR_TOK_SI32;
}

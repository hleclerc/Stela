#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Continue.h"

Ast_Continue::Ast_Continue( int off ) : Ast( off ) {
    n = 1;
}

void Ast_Continue::write_to_stream( Stream &os, int nsp ) const {
    os << "Continue";
}


void Ast_Continue::_get_info( IrWriter *aw ) const {
    aw->data << n;
}

PI8 Ast_Continue::_tok_number() const {
    return IR_TOK_CONTINUE;
}

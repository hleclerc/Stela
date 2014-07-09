#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_Break.h"

Ast_Break::Ast_Break( int off ) : Ast( off ) {
    n = 1;
}

void Ast_Break::write_to_stream( Stream &os, int nsp ) const {
    os << "Break[" << n << "]";
}

void Ast_Break::_get_info( AstWriter *aw ) const {
    TODO;
}

PI8 Ast_Break::_tok_number() const {
    return IR_TOK_BREAK;
}

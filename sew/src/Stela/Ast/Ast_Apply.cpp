#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_Apply.h"

Ast_Apply::Ast_Apply( int off ) : Ast_Call( off ) {
}

void Ast_Apply::_get_info( AstWriter *aw ) const {
    TODO;
}

PI8 Ast_Apply::_tok_number() const {
    return IR_TOK_APPLY;
}


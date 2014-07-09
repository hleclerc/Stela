#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_Select.h"

Ast_Select::Ast_Select( int off ) : Ast_Call( off ) {
}

PI8 Ast_Select::_tok_number() const {
    return IR_TOK_SELECT;
}

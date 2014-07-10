#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_New.h"

Ast_New::Ast_New( int off ) : Ast_Call( off ) {
}

PI8 Ast_New::_tok_number() const {
    return IR_TOK_NEW;
}

#include "../Ir/Numbers.h"
#include "IrWriter.h"

#include "Ast_Variable.h"
#include "Ast_Apply.h"

Ast_Apply::Ast_Apply( int off ) : Ast_Call( off ) {
}

PI8 Ast_Apply::_tok_number() const {
    return IR_TOK_APPLY;
}


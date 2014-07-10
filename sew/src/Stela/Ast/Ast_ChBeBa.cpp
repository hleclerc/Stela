#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_ChBeBa.h"

Ast_ChBeBa::Ast_ChBeBa( int off ) : Ast_Call( off ) {
}

PI8 Ast_ChBeBa::_tok_number() const {
    return IR_TOK_CHBEBA;
}

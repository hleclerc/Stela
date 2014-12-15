#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Select.h"

Ast_Select::Ast_Select( int off ) : Ast_Call( off ) {
}

Expr Ast_Select::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in", false, __FILE__, __LINE__ );
}

PI8 Ast_Select::_tok_number() const {
    return IR_TOK_SELECT;
}

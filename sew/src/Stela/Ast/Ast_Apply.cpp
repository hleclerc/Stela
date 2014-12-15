#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"

#include "Ast_Variable.h"
#include "Ast_Apply.h"

Ast_Apply::Ast_Apply( int off ) : Ast_Call( off ) {
}

Expr Ast_Apply::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in", false, __FILE__, __LINE__ );
}

PI8 Ast_Apply::_tok_number() const {
    return IR_TOK_APPLY;
}


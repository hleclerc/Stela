#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_ChBeBa.h"

Ast_ChBeBa::Ast_ChBeBa( int off ) : Ast_Call( off ) {
}

Expr Ast_ChBeBa::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in" );
}

PI8 Ast_ChBeBa::_tok_number() const {
    return IR_TOK_CHBEBA;
}

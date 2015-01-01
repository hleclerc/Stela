#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_New.h"

Ast_New::Ast_New( const char *src, int off ) : Ast_Call( src, off ) {
}

Expr Ast_New::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in", false, __FILE__, __LINE__ );
}

PI8 Ast_New::_tok_number() const {
    return IR_TOK_NEW;
}

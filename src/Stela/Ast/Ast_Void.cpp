#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Void.h"

Ast_Void::Ast_Void( const char *src, int off ) : Ast( src, off ) {
}

void Ast_Void::write_to_stream( Stream &os, int nsp ) const {
    os << "Void";
}

Expr Ast_Void::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in", false, __FILE__, __LINE__ );
}

PI8 Ast_Void::_tok_number() const {
    return IR_TOK_VOID;
}

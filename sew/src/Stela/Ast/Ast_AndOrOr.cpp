#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "Ast_AndOrOr.h"
#include "IrWriter.h"

Ast_AndOrOr::Ast_AndOrOr( const char *src, int off, bool want_and ) : Ast( src, off ), want_and( want_and ) {
}

void Ast_AndOrOr::write_to_stream( Stream &os, int nsp ) const {
    os << ( want_and ? "and" : "or" );
}

Expr Ast_AndOrOr::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in", false, __FILE__, __LINE__ );
}

void Ast_AndOrOr::_get_info( IrWriter *aw ) const {
    aw->push_delayed_parse( args[ 0 ].ptr() );
    aw->push_delayed_parse( args[ 1 ].ptr() );
}

PI8 Ast_AndOrOr::_tok_number() const {
    return want_and ? IR_TOK_AND : IR_TOK_OR;
}


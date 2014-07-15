#include "../Ir/Numbers.h"
#include "Ast_AndOrOr.h"
#include "IrWriter.h"

Ast_AndOrOr::Ast_AndOrOr( int off, bool want_and ) : Ast( off ), want_and( want_and ) {
}

void Ast_AndOrOr::write_to_stream( Stream &os, int nsp ) const {
    os << ( want_and ? "and" : "or" );
}

void Ast_AndOrOr::_get_info( IrWriter *aw ) const {
    aw->push_delayed_parse( args[ 0 ].ptr() );
    aw->push_delayed_parse( args[ 1 ].ptr() );
}

PI8 Ast_AndOrOr::_tok_number() const {
    return want_and ? IR_TOK_AND : IR_TOK_OR;
}


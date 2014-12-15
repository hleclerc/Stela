#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_While.h"

Ast_While::Ast_While( int off ) : Ast( off ) {
}

void Ast_While::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    std::set<String> oavail = avail;
    ok->get_potentially_needed_ext_vars( res, oavail );

    if ( ko ) {
        std::set<String> kavail = avail;
        ko->get_potentially_needed_ext_vars( res, kavail );
    }
}

void Ast_While::write_to_stream( Stream &os, int nsp ) const {
    os << "while";

    ok->write_to_stream( os, nsp + 2 );

    if ( ko ) {
        os << "\n" << String( nsp, ' ' ) << "else ";
        ko->write_to_stream( os, nsp + 2 );
    }
}

void Ast_While::_get_info( IrWriter *aw ) const {
    aw->push_delayed_parse( ok.ptr() );
    aw->push_delayed_parse( ko.ptr() );
}

Expr Ast_While::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in", false, __FILE__, __LINE__ );
}

PI8 Ast_While::_tok_number() const {
    return IR_TOK_WHILE;
}


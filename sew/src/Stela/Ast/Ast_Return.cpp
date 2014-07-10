#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Return.h"

Ast_Return::Ast_Return( int off ) : Ast( off ) {
}

void Ast_Return::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    val->get_potentially_needed_ext_vars( res, avail );
}

void Ast_Return::write_to_stream( Stream &os, int nsp ) const {
    os << "return ";
    val->write_to_stream( os, nsp + 2 );
}


void Ast_Return::_get_info( IrWriter *aw ) const {
    aw->push_delayed_parse( val.ptr() );
}

PI8 Ast_Return::_tok_number() const {
    return IR_TOK_RETURN;
}

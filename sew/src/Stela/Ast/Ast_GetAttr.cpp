#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_GetAttr.h"

Ast_GetAttr::Ast_GetAttr( int off ) : Ast( off ) {
}

void Ast_GetAttr::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    obj->get_potentially_needed_ext_vars( res, avail );
}

void Ast_GetAttr::write_to_stream( Stream &os, int nsp ) const {
    obj->write_to_stream( os, nsp + 6 );

    if ( ddo )
        os << "::";
    else if ( ptr )
        os << "->";
    else
        os << ".";

    if ( ask )
        os << "?";

    os << name;
}


void Ast_GetAttr::_get_info( IrWriter *aw ) const {
    aw->push_delayed_parse( obj.ptr() );
    aw->push_nstring( name );
}

PI8 Ast_GetAttr::_tok_number() const {
    if ( ddo )
        return IR_TOK_GET_ATTR_PA;
    if ( ptr )
        return ask ? IR_TOK_GET_ATTR_PTR_ASK : IR_TOK_GET_ATTR_PTR;
    return ask ? IR_TOK_GET_ATTR_ASK : IR_TOK_GET_ATTR;
}

#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_GetAttr.h"

Ast_GetAttr::Ast_GetAttr( int off ) : Ast( off ) {
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


void Ast_GetAttr::_get_info( AstWriter *aw ) const {
    TODO;
}

PI8 Ast_GetAttr::_tok_number() const {
    if ( ddo )
        return IR_TOK_GET_ATTR_PA;
    if ( ptr )
        return ask ? IR_TOK_GET_ATTR_PTR_ASK : IR_TOK_GET_ATTR_PTR;
    return ask ? IR_TOK_GET_ATTR_ASK : IR_TOK_GET_ATTR;
}

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


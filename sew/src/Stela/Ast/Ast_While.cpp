#include "Ast_While.h"

Ast_While::Ast_While( int off ) : Ast( off ) {
}

void Ast_While::write_to_stream( Stream &os, int nsp ) const {
    os << "while";

    ok->write_to_stream( os, nsp + 2 );

    if ( ko ) {
        os << "\n" << String( nsp, ' ' ) << "else ";
        ko->write_to_stream( os, nsp + 2 );
    }
}


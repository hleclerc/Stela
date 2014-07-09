#include "Ast_If.h"

Ast_If::Ast_If( int off ) : Ast( off ) {
}

void Ast_If::write_to_stream( Stream &os, int nsp ) const {
    os << "if ";

    cond->write_to_stream( os, nsp + 6 );

    ok->write_to_stream( os, nsp + 2 );

    if ( ko ) {
        os << "\n" << String( nsp, ' ' ) << "else ";
        ko->write_to_stream( os, nsp + 2 );
    }
}


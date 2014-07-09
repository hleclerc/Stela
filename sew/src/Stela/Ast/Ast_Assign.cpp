#include "Ast_Assign.h"

Ast_Assign::Ast_Assign( int off ) : Ast( off ) {
}

void Ast_Assign::write_to_stream( Stream &os, int nsp ) const {
    os << name;
    if ( stat )
        os << "static ";
    if ( cons )
        os << "const ";
    if ( type )
        os << " ~= ";
    else
        os << " := ";
    if ( ref )
        os << "ref ";
    val->write_to_stream( os, nsp );
}


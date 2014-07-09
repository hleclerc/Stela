#include "Ast_Void.h"

Ast_Void::Ast_Void( int off ) : Ast( off ) {
}

void Ast_Void::write_to_stream( Stream &os, int nsp ) const {
    os << "Void";
}


#include "Ast_Import.h"

Ast_Import::Ast_Import( int off ) : Ast( off ) {
}

void Ast_Import::write_to_stream( Stream &os, int nsp ) const {
    os << "import";
    for( const String &f : files )
        os << " " << f;
}


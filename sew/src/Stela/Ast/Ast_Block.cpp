#include "Ast_Block.h"

Ast_Block::Ast_Block( int off ) : Ast( off ) {
}

void Ast_Block::write_to_stream( Stream &os, int nsp ) const {
    for( int i = 0; i < lst.size(); ++i )
        lst[ i ]->write_to_stream( os << "\n" << String( nsp + 2, ' ' ), nsp + 2 );
}


#include "Ast_Call.h"

Ast_Call::Ast_Call( int off ) : Ast( off ) {
}

void Ast_Call::write_to_stream( Stream &os, int nsp ) const {
    write_name( os );
    f->write_to_stream( os << " ", nsp + 6 );
    for( int i = 0; i < args.size(); ++i ) {
        os << "\n" << String( nsp + 2, ' ' );
        if ( i - ( args.size() - names.size() ) >= 0 )
            os << names[ i - ( args.size() - names.size() ) ] << "=";
        args[ i ]->write_to_stream( os, nsp + 2 );
    }
}


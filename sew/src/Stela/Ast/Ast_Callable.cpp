#include "Ast_Callable.h"

Ast_Callable::Ast_Callable( int off ) : Ast( off ) {
}

void Ast_Callable::write_to_stream( Stream &os, int nsp ) const {
    write_callable_type( os );
    os << " " << name << "(";
    if ( self_as_arg )
        os << " self";
    for( int i = 0; i < arguments.size(); ++i ) {
        if ( i or self_as_arg )
            os << ",";
        os << " " << arguments[ i ];
    }
    if ( varargs ) {
        if ( arguments.size() or self_as_arg )
            os << ",";
        os << "varargs";
    }
    if ( arguments.size() or self_as_arg or varargs )
        os << " ";
    os << ")";

    if ( condition )
        condition->write_to_stream( os << " when ", nsp + 2 );
    if ( return_type )
        return_type->write_to_stream( os << " : ", nsp + 2 );
    if ( pertinence )
        pertinence->write_to_stream( os << " pertinence ", nsp + 2 );

    block->write_to_stream( os << " ", nsp + 2 );
}


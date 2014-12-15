#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Call.h"

Ast_Call::Ast_Call( int off ) : Ast( off ) {
}

void Ast_Call::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    f->get_potentially_needed_ext_vars( res, avail );
    for( int i = 0; i < args.size(); ++i )
        args[ i ]->get_potentially_needed_ext_vars( res, avail );
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

Expr Ast_Call::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in" );
}

void Ast_Call::_get_info( IrWriter *aw ) const {
    int nn = names.size(), nu = args.size() - nn;

    aw->push_delayed_parse( f.ptr() );

    aw->data << nu;
    for( int i = 0; i < nu; ++i )
        aw->push_delayed_parse( args[ i ].ptr() );

    aw->data << nn;
    for( int i = 0; i < nn; ++i ) {
        aw->push_nstring( names[ i ] );
        aw->push_delayed_parse( args[ nu + i ].ptr() );
    }
}

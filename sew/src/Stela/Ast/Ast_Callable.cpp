#include "../Ssa/ParsingContext.h"
#include "../Ir/CallableFlags.h"
#include "../Ir/Numbers.h"
#include "Ast_Callable.h"
#include "IrWriter.h"
#include <limits>

Ast_Callable::Ast_Callable( const char *src, int off ) : Ast( src, off ) {
}

void Ast_Callable::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    // function name
    avail.insert( name );

    // new scope
    std::set<String> navail = avail;

    // arguments
    for( int i = 0, b = arguments.size() - default_values.size(); i < arguments.size(); ++i ) {
        navail.insert( arguments[ i ] );
        if ( i >= b )
            default_values[ i - b ]->get_potentially_needed_ext_vars( res, navail );
    }
    if ( self_as_arg )
        navail.insert( "self" );
    if ( varargs )
        navail.insert( "varargs" );

    // cond, ...
    if ( condition )
        condition->get_potentially_needed_ext_vars( res, navail );
    if ( pertinence )
        pertinence->get_potentially_needed_ext_vars( res, navail );

    // sub cases
    _get_potentially_needed_ext_vars( res, navail );

    // block
    block->get_potentially_needed_ext_vars( res, navail );
}


void Ast_Callable::prep_get_potentially_needed_ext_vars( std::set<String> &avail ) const {
    avail.insert( name );
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
    if ( pertinence )
        pertinence->write_to_stream( os << " pertinence ", nsp + 2 );

    block->write_to_stream( os << " ", nsp + 2 );
}

double Ast_Callable::default_pertinence() const {
    return def_pert_num / double( def_pert_den );
}

int Ast_Callable::min_nb_args() const {
    return arguments.size() - default_values.size();
}

int Ast_Callable::max_nb_args() const {
    if ( varargs )
        return std::numeric_limits<int>::max();
    return arguments.size();
}

Expr Ast_Callable::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in", false, __FILE__, __LINE__ );
}

void Ast_Callable::_get_info( IrWriter *aw ) const {
    aw->push_nstring( name );

    aw->data << bool( self_as_arg      ) * IR_SELF_AS_ARG +
                bool( abstract         ) * IR_ABSTRACT +
                bool( varargs          ) * IR_VARARGS +
                bool( pertinence       ) * IR_HAS_COMPUTED_PERT +
                bool( condition        ) * IR_HAS_CONDITION +
                bool( def_pert_num < 0 ) * IR_NEG_PERT +
                _spec_flags();

    aw->data << arguments.size();
    aw->data << default_values.size();
    if ( not pertinence ) {
        aw->data << abs( def_pert_num );
        aw->data << def_pert_den;
    }
    for( int i = 0; i < arguments.size(); ++i ) {
        // name
        aw->push_nstring( arguments[ i ] );

        // constraints
        aw->data << arg_constraints[ i ].size();
        for( int j = 0; j < arg_constraints[ i ].size(); ++j )
            aw->push_nstring( arg_constraints[ i ][ j ] );
    }
    for( int i = 0; i < default_values.size(); ++i )
        aw->push_delayed_parse( default_values[ i ].ptr() );
    if ( pertinence )
        aw->push_delayed_parse( pertinence.ptr() );
    if ( condition )
        aw->push_delayed_parse( condition.ptr() );

    // block
    aw->push_delayed_parse( block.ptr() );

    // catched var data
    aw->push_potential_catched_vars_from( this );
}


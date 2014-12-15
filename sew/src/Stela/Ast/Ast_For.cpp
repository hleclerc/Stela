#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_For.h"

Ast_For::Ast_For( int off ) : Ast( off ) {
}

void Ast_For::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    std::set<String> navail = avail;

    for( int i = 0; i < objects.size(); ++i )
        objects[ i ]->get_potentially_needed_ext_vars( res, navail );

    for( int i = 0; i < names.size(); ++i )
        navail.insert( names[ i ] );
    block->get_potentially_needed_ext_vars( res, navail );
}

void Ast_For::write_to_stream( Stream &os, int nsp ) const {
    os << "For";
}


void Ast_For::_get_info( IrWriter *aw ) const {
    // names
    aw->data << names.size();
    for( int i = 0; i < names.size(); ++i )
        aw->push_nstring( names[ i ] );

    // obj
    aw->data << objects.size();
    for( int i = 0; i < objects.size(); ++i )
        aw->push_delayed_parse( objects[ i ].ptr() );

    // catched
    std::set<String> avail;
    for( int i = 0; i < names.size(); ++i )
        avail.insert( names[ i ] );
    aw->push_potential_catched_vars_from( block.ptr(), avail );

    // block
    aw->push_delayed_parse( block.ptr() );
}

Expr Ast_For::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in" );
}

PI8 Ast_For::_tok_number() const {
    return IR_TOK_FOR;
}

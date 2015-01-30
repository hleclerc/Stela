#include "../Ssa/BlockWithCatchedVars.h"
#include "../Ssa/ParsingContext.h"
#include "../Ssa/Room.h"
#include "../Ssa/Cst.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_For.h"

Ast_For::Ast_For( const char *src, int off ) : Ast( src, off ) {
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
    if ( objects.size() != 1 )
        return context.ret_error( "TODO", false, __FILE__, __LINE__ );
    Expr o = objects[ 0 ]->parse_in( context );

    std::set<String> res, avail;
    for( String n : names )
        avail.insert( n );

    block->get_potentially_needed_ext_vars( res, avail );

    BlockWithCatchedVars *blk = new BlockWithCatchedVars( block, &context, res.size() );
    for( String n : res ) {
        if ( Expr var = context.get_var( n, false ) ) {
            // if the same variable is in the main scope -> we do not store it again (we will have access to it after that)
            if ( ip->main_parsing_context->get_var( n, false ).equal( var ) )
                continue;
            blk->catched_vars.push_named( n, var );
        }
    }
    PRINT( blk->catched_vars );

    if ( names.size() != 1 )
        return context.ret_error( "TODO", false, __FILE__, __LINE__ );
    blk->name_args << names[ 0 ];

    Expr blk_expr = room( cst( ip->type_Block, 64, &blk ) );
    return context.apply( context.get_attr( o, "__for__" ), 1, &blk_expr );
}

PI8 Ast_For::_tok_number() const {
    return IR_TOK_FOR;
}

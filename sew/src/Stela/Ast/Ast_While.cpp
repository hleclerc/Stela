#include "../Ssa/ParsingContext.h"
#include "../Ssa/UnknownInst.h"
#include "../Ssa/IpSnapshot.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_While.h"

Ast_While::Ast_While( int off ) : Ast( off ) {
}

void Ast_While::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    std::set<String> oavail = avail;
    ok->get_potentially_needed_ext_vars( res, oavail );

    if ( ko ) {
        std::set<String> kavail = avail;
        ko->get_potentially_needed_ext_vars( res, kavail );
    }
}

void Ast_While::write_to_stream( Stream &os, int nsp ) const {
    os << "while";

    ok->write_to_stream( os, nsp + 2 );

    if ( ko ) {
        os << "\n" << String( nsp, ' ' ) << "else ";
        ko->write_to_stream( os, nsp + 2 );
    }
}

void Ast_While::_get_info( IrWriter *aw ) const {
    aw->push_delayed_parse( ok.ptr() );
    aw->push_delayed_parse( ko.ptr() );
}

Expr Ast_While::_parse_in( ParsingContext &context ) const {
    // watch modified variables
    IpSnapshot nsv( ip->ip_snapshot );

    // we repeat until there are no external modified values
    int ne = ip->error_list.size();
    std::map<Expr,Expr> unknowns;
    for( unsigned old_nsv_size = 0, cpt = 0; ; old_nsv_size = nsv.rooms.size(), ++cpt ) {
        if ( cpt == 100 )
            return context.ret_error( "infinite loop during while parsing" );

        ParsingContext wh_scope( &context, 0, "while_" + to_string( this ) );
        Expr cont_var( true ); wh_scope.cont = &cont_var;
        ok->parse_in( wh_scope );

        if ( ne != ip->error_list.size() )
            return Expr();

        // if no new modified variables
        if ( old_nsv_size == nsv.rooms.size() )
            break;

        // replace each modified variable to a new unknown variables
        // (to avoid simplifications during the next round)
        for( std::pair<Inst *const,Expr> &it : nsv.rooms ) {
            Expr unk = unknown_inst( it.second->type(), cpt );
            unknowns[ it.first ] = unk;
            const_cast<Inst *>( it.first )->set( unk, true );
        }
    }

    PRINT( unknowns.size() );
    for( auto unk: unknowns )
        std::cout << unk.first << " " << unk.second << std::endl;

    return context.ret_error( "TODO: _parse_in", false, __FILE__, __LINE__ );
}

PI8 Ast_While::_tok_number() const {
    return IR_TOK_WHILE;
}


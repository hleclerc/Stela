#include "../Ssa/ParsingContext.h"
#include "../Ssa/Select.h"
#include "../Ssa/Op.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_If.h"

Ast_If::Ast_If( int off ) : Ast( off ) {
}

void Ast_If::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    std::set<String> cavail = avail;
    cond->get_potentially_needed_ext_vars( res, cavail );

    std::set<String> oavail = cavail;
    ok->get_potentially_needed_ext_vars( res, oavail );

    if ( ko ) {
        std::set<String> kavail = cavail;
        ko->get_potentially_needed_ext_vars( res, kavail );
    }
}

void Ast_If::write_to_stream( Stream &os, int nsp ) const {
    os << "if ";

    cond->write_to_stream( os, nsp + 6 );

    ok->write_to_stream( os, nsp + 2 );

    if ( ko ) {
        os << "\n" << String( nsp, ' ' ) << "else ";
        ko->write_to_stream( os, nsp + 2 );
    }
}


void Ast_If::_get_info( IrWriter *aw ) const {
    aw->push_delayed_parse( cond.ptr() );
    aw->push_delayed_parse( ok.ptr() );
    aw->push_delayed_parse( ko.ptr() );
}

Expr Ast_If::_parse_in( ParsingContext &context ) const {
    Expr cond_if = cond->parse_in( context );
    if ( cond_if.error() )
        return cond_if;

    // bool conversion
    if ( cond_if->ptype() != ip->type_Bool ) {
        cond_if = context.apply( context.get_var( "Bool" ), 1, &cond_if );
        if ( cond_if.error() )
            return cond_if;
    }

    // simplified expression
    Expr expr = simplified_cond( cond_if->get( context.cond ) );

    // known value
    if ( expr->always( true ) ) {
        ParsingContext if_scope( &context, 0, "if_" + to_string( ok ) );
        return ok->parse_in( if_scope );
    }
    if ( expr->always( false ) ) {
        if ( ko ) {
            ParsingContext if_scope( &context, 0, "fi_" + to_string( ko ) );
            return ko->parse_in( if_scope );
        }
        return ip->void_var();
    }

    Expr res_ok;
    if ( ok ) {
        ParsingContext if_scope( &context, 0, "if_" + to_string( ok ) );
        if_scope.cond = expr;
        res_ok = ok->parse_in( if_scope );
    }

    Expr res_ko;
    if ( ko ) {
        ParsingContext fi_scope( &context, 0, "fi_" + to_string( ko ) );
        fi_scope.cond = not_boolean( expr );
        res_ko =  ko->parse_in( fi_scope );
    }

    return select( expr, res_ok, res_ko );
}

PI8 Ast_If::_tok_number() const {
    return IR_TOK_IF;
}

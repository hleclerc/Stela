#include "../Ssa/ParsingContext.h"
#include "../Ssa/UnknownInst.h"
#include "../Ssa/IpSnapshot.h"
#include "../Ssa/GetNout.h"
#include "../Ssa/While.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_While.h"

Ast_While::Ast_While( const char *src, int off ) : Ast( src, off ) {
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
        if ( cpt == 20 )
            return context.ret_error( "infinite loop during while parsing" );

        ParsingContext wh_scope( &context, 0, "while_" + to_string( ok ) );
        wh_scope.cont = true;
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
        nsv.undo_parsing_contexts();
    }

    // corr table (output number -> input number)
    // -> find if Unknown inst are used to compute the outputs
    ++Inst::cur_op_id;
    for( std::pair<Inst *const,Expr> &it : nsv.rooms )
        const_cast<Inst *>( it.first )->get( context.cond )->mark_children();
    int cpt = 0;
    Vec<int> corr;
    Vec<Type *> inp_types;
    for( std::pair<Inst *const,Expr> &it : nsv.rooms ) {
        if ( unknowns[ it.first ]->op_id == Inst::cur_op_id ) {
            corr << cpt++;
            inp_types << it.second->type();
        } else
            corr << -1;
    }

    // prepare a while inp (for initial values of variables modified in the loop)
    Expr winp = while_inp( inp_types );

    // set winp[...] as initial values of modified variables
    cpt = 0;
    for( std::pair<Inst *const,Expr> &it : nsv.rooms ) {
        int num_inp = corr[ cpt++ ];
        if ( num_inp >= 0 )
            const_cast<Inst *>( it.first )->set( get_nout( winp, num_inp ), true );
        else
            const_cast<Inst *>( it.first )->set( it.second, true );
    }
    nsv.undo_parsing_contexts();

    // relaunch the while inst
    ParsingContext wh_scope( &context, 0, "while_" + to_string( ok ) );
    wh_scope.cont = true;
    ok->parse_in( wh_scope );

    if ( wh_scope.cont->always( false ) ) {
        ip->ip_snapshot = nsv.prev;
    } else {
        // make the while instruction
        Vec<Expr> out_exprs;
        for( std::pair<Inst *const,Expr> &it : nsv.rooms )
            out_exprs << const_cast<Inst *>( it.first )->get( context.cond );
        PRINT( wh_scope.cont );
        Expr wout = while_out( out_exprs, wh_scope.cont );

        cpt = 0;
        Vec<Expr> inp_exprs;
        for( std::pair<Inst *const,Expr> &it : nsv.rooms )
            if ( corr[ cpt++ ] >= 0 )
                inp_exprs << it.second->simplified( context.cond );
        Expr wins = while_inst( inp_exprs, winp, wout, corr );

        ip->ip_snapshot = nsv.prev;

        // replace changed variable by while_inst outputs
        cpt = 0;
        for( std::pair<Inst *const,Expr> &it : nsv.rooms )
            const_cast<Inst *>( it.first )->set( get_nout( wins, cpt++ ), context.cond );
    }

    // break(s) to transmit ?
    for( ParsingContext::RemBreak rb : wh_scope.rem_breaks )
        context.BREAK( rb.count, rb.cond );

    return ip->void_var();
}

PI8 Ast_While::_tok_number() const {
    return IR_TOK_WHILE;
}


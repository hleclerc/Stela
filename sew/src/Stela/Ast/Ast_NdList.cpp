#include "../Ssa/ParsingContext.h"
#include "../Ssa/ReplBits.h"
#include "../Ssa/Class.h"
#include "../Ssa/Room.h"
#include "../Ssa/Type.h"
#include "../Ssa/Cst.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_NdList.h"

Ast_NdList::Ast_NdList( const char *src, int off ) : Ast( src, off ) {
}

void Ast_NdList::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    for( int i = 0; i < lst.size(); ++i )
        lst[ i ]->get_potentially_needed_ext_vars( res, avail );
}

void Ast_NdList::write_to_stream( Stream &os, int nsp ) const {
    os << "nb_dim=" << nb_dim << " sizes=" << sizes;
    for( int i = 0; i < lst.size(); ++i )
        lst[ i ]->write_to_stream( os << "\n" << String( nsp + 2, ' ' ), nsp + 2 );
}


void Ast_NdList::_get_info( IrWriter *aw ) const {
    TODO;
}

static Type *make_type_nd_var_list( Vec<Expr> vars, int o = 0 ) {
    if ( o == vars.size() )
        return ip->type_NdListItemEnd;
    Vec<Expr> parms;
    parms << ip->pc->type_expr( vars[ o ]->ptype() );
    parms << ip->pc->type_expr( make_type_nd_var_list( vars, o + 1 ) );
    return ip->class_NdListItemBeg->type_for( parms );
}

Expr Ast_NdList::_make_nd_var_list( Vec<Expr> vars ) const {
    Type *type = make_type_nd_var_list( vars );

    Vec<Expr> size_exprs;
    for( int s : sizes )
        size_exprs << room( s );

    Vec<Expr> args;
    args << ip->pc->type_expr( type );
    args << room( nb_dim );
    args << ip->pc->make_static_list( size_exprs );
    Type *rt = ip->class_NdList->type_for( args );

    Expr res = cst( rt, rt->size(), 0 );
    PRINT( vars );
    for( int i = 0; i < vars.size(); ++i ) {
        res = repl_bits( res, 8 * sizeof( ST ) * i, vars[ i ] );
    }
    PRINT( res );
    return room( res );
}


Expr Ast_NdList::_parse_in( ParsingContext &context ) const {
    Vec<Expr> vars( Rese(), lst.size() );
    for( Past p : lst )
        vars << p->parse_in( context );
    return _make_nd_var_list( vars );
}

PI8 Ast_NdList::_tok_number() const {
    return IR_TOK_LIST;
}

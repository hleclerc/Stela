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

void Ast_NdList::get_items_StaticListItem( Vec<Expr> &res, Expr static_list ) {
    if ( not static_list ) return;
    Expr sl = static_list->get( ip->pc->cond );
    if ( not sl ) return;

    Type *st = sl->type();
    if ( st->orig == ip->class_StaticListItemEnd )
        return;
    if ( st->orig != ip->class_StaticListItemBeg )
        return ip->pc->disp_error( "not a StaticListItem... ?" );
    res << st->parameters[ 0 ]->get( ip->pc->cond );
    get_items_StaticListItem( res, st->parameters[ 1 ] );
}

void Ast_NdList::get_items_NdListItem( Vec<Expr> &res, Expr nd_list_item ) {
    if ( not nd_list_item ) return;
    Expr nl = nd_list_item->get( ip->pc->cond );
    if ( not nl ) return;

    Type *nt = nl->type();
    if ( nt->orig == ip->class_NdListItemEnd )
        return;

    if ( nt->orig != ip->class_NdListItemBeg )
        return ip->pc->disp_error( "not a StaticListItem... ?" );

    // NdListItemBeg
    Type::Attr *data_attr = nt->find_attr( "data" );
    if ( not data_attr )
        return ip->pc->disp_error( "...", false, __FILE__, __LINE__ );
    res << nt->attr_expr( nd_list_item, *data_attr )->get( ip->pc->cond );

    Type::Attr *next_attr = nt->find_attr( "next" );
    if ( not next_attr )
        return ip->pc->disp_error( "...", false, __FILE__, __LINE__ );
    get_items_NdListItem( res, nt->attr_expr( nd_list_item, *next_attr ) );
}

void Ast_NdList::get_items( int &nb_dim, Vec<int> &sizes, Vec<Expr> &values, Expr nd_list ) {
    if ( not nd_list )
        return;
    Expr nd = nd_list->get( ip->pc->cond );
    if ( not nd )
        return;

    Type *nt = nd->type();
    ASSERT( nt->orig == ip->class_NdList, "..." );

    // nb_dim
    if ( not nt->parameters[ 1 ]->get( ip->pc->cond )->get_val( &nb_dim, ip->type_SI32 ) )
        return ip->pc->disp_error( "...", false, __FILE__, __LINE__ );

    // sizes
    Vec<Expr> expr_sizes;
    get_items_StaticListItem( expr_sizes, nt->parameters[ 2 ] );
    for( Expr e : expr_sizes )
        if ( not e->get_val( sizes.push_back(), ip->type_SI32 ) )
            return ip->pc->disp_error( "...", false, __FILE__, __LINE__ );

    // values
    Type::Attr *attr = nt->find_attr( "values" );
    if ( not attr )
        return ip->pc->disp_error( "...", false, __FILE__, __LINE__ );
    get_items_NdListItem( values, nt->attr_expr( nd_list, *attr ) );
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
    for( int i = 0; i < vars.size(); ++i )
        res = repl_bits( res, 8 * sizeof( ST ) * i, vars[ i ] );
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

#include "../Inst/Phi.h"
#include "RefPhi.h"

RefPhi::RefPhi( const Expr &cond, Ptr<Ref> ok, Ptr<Ref> ko ) : cond( cond ), ok( ok ), ko( ko ) {
}

void RefPhi::write_to_stream( Stream &os ) const {
    os << "select( " << cond << ", " << ok << ", " << ko << ")";
}

bool RefPhi::indirect_set( const Var &src, Scope *set_scope, const Expr &sf, int off, Expr ext_cond ) {
    //Expr res = setval( var.expr(), expr, beg );
    //set_scope->set( var, res, sf, off, ext_cond );
    //    if ( RefPointerOn *rpo = dcast( src.data->ptr.ptr() ) ) {
    //        set_scope->_set_refexpr( var, , sf, off, ext_cond );
    //    } else
    TODO;
    return true;
}

bool RefPhi::contains_var_referenced_more_than_one_time() const {
    // return var.;
    TODO;
    return true;
}

Expr RefPhi::expr() const {
    return phi( cond, ok->expr(), ko->expr() );
}

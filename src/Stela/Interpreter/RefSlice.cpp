#include "../Inst/SetVal.h"
#include "../Inst/Slice.h"
#include "RefSlice.h"
#include "RefExpr.h"
#include "Scope.h"

RefSlice::RefSlice( const Var &var, int beg, int end ) : var( var ), beg( beg ), end( end ) {
}

Expr RefSlice::expr() const {
    return slice( var.expr(), beg, end );
}

Ptr<Ref> RefSlice::copy() {
    return new RefExpr( expr() );
}


bool RefSlice::contains_var_referenced_more_than_one_time() const {
    return var.referenced_more_than_one_time();
}


bool RefSlice::indirect_set( const Var &src, Scope *set_scope, const Expr &sf, int off, Expr ext_cond ) {
    Expr expr = set_scope->simplified_expr( src, sf, off );
    ASSERT( expr.size_in_bits() == end - beg, "..." );

    Expr res = setval( set_scope->simplified_expr( var, sf, off ), expr, beg );
    set_scope->set( var, Var( var.type, res ), sf, off, ext_cond );

    return true;
}

void RefSlice::write_to_stream( Stream &os ) const {
    os << "slice(" << var << "," << beg << "," << end << ")";
}

#include "../Inst/SetVal.h"
#include "../Inst/Slice.h"
#include "RefSliceUnk.h"
#include "RefExpr.h"
#include "Scope.h"

RefSliceUnk::RefSliceUnk( const Var &var, const Expr &beg, int len ) : var( var ), beg( beg ), len( len ) {
}

Expr RefSliceUnk::expr() const {
    return slice( var.expr(), beg, len );
}

Ptr<Ref> RefSliceUnk::copy() {
    return new RefExpr( expr() );
}

bool RefSliceUnk::contains_var_referenced_more_than_one_time() const {
    return var.referenced_more_than_one_time();
}

bool RefSliceUnk::indirect_set( const Var &src, Scope *set_scope, const Expr &sf, int off, Expr ext_cond ) {
    Expr expr = set_scope->simplified_expr( src, sf, off );

    ASSERT( expr.size_in_bits() == len, "..." );
    Expr res = setval( var.expr(), expr, beg );
    set_scope->set( var, Var( var.type, res ), sf, off, ext_cond );
    return true;
}

void RefSliceUnk::write_to_stream( Stream &os ) const {
    os << "slice(" << var << "," << beg << ",len=" << len << ")";
}

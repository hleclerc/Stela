#include "../Inst/Concat.h"
#include "../Inst/Slice.h"
#include "RefSlice.h"
#include "Scope.h"

RefSlice::RefSlice( const Var &var, int beg, int end ) : var( var ), beg( beg ), end( end ) {
}

Expr RefSlice::expr() const {
    return slice( var.expr(), beg, end );
}

bool RefSlice::indirect_set( Expr expr, Scope *set_scope, const Expr &sf, int off, Expr ext_cond ) {
    ASSERT( expr.size_in_bits() == end - beg, "..." );
    Expr orig = var.expr();
    Expr res = concat( slice( orig, 0, beg ), expr, slice( orig, end, orig.size_in_bits() ) );
    set_scope->set( var, res, sf, off, ext_cond );
    return true;
}

void RefSlice::write_to_stream( Stream &os ) const {
    os << "slice(" << var << "," << beg << "," << end << ")";
}

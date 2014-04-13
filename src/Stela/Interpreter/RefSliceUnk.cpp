#include "../Inst/SetVal.h"
#include "../Inst/Slice.h"
#include "RefSliceUnk.h"
#include "Scope.h"

RefSliceUnk::RefSliceUnk( const Var &var, const Expr &beg, int len ) : var( var ), beg( beg ), len( len ) {
}

Expr RefSliceUnk::expr() const {
    return slice( var.expr(), beg, len );
}

bool RefSliceUnk::indirect_set( Expr expr, Scope *set_scope, const Expr &sf, int off, Expr ext_cond ) {
    ASSERT( expr.size_in_bits() == len, "..." );
    Expr res = setval( var.expr(), expr, beg );
    set_scope->set( var, res, sf, off, ext_cond );
    return true;
}

void RefSliceUnk::write_to_stream( Stream &os ) const {
    os << "slice(" << var << "," << beg << ",len=" << len << ")";
}

#include "../Inst/Concat.h"
#include "../Inst/Slice.h"
#include "RefSlice.h"

RefSlice::RefSlice( const Var &var, int beg, int end ) : var( var ), beg( beg ), end( end ) {
}

Expr RefSlice::expr() const {
    return slice( var.expr(), beg, end );
}

void RefSlice::set( Expr expr ) {
    ASSERT( expr.size_in_bits() == end - beg, "..." );

    Expr orig = var.expr();
    Expr nexpr = concat( slice( orig, 0, beg ), expr, slice( orig, end, orig.size_in_bits() ) );
    var.data->ptr->set( nexpr );
}

void RefSlice::write_to_stream( Stream &os ) const {
    os << "slice(" << var << "," << beg << "," << end << ")";
}

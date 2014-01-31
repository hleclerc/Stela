#include "../Inst/Slice.h"
#include "RefSlice.h"

RefSlice::RefSlice( const Var &var, int beg, int end ) : var( var ), beg( beg ), end( end ) {
}

Expr RefSlice::expr() const {
    return slice( var.expr(), beg, end );
}

void RefSlice::set( Expr expr ) {
    TODO;
}

void RefSlice::write_to_stream( Stream &os ) const {
    os << "slice(" << var << "," << beg << "," << end << ")";
}

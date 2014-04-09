#include "../Inst/SetVal.h"
#include "../Inst/Slice.h"
#include "RefSliceUnk.h"
#include "Scope.h"

RefSliceUnk::RefSliceUnk( const Var &var, const Expr &beg, int len ) : var( var ), beg( beg ), len( len ) {
}

Expr RefSliceUnk::expr() const {
    return slice( var.expr(), beg, len );
}

void RefSliceUnk::set( Expr expr, Scope *set_scope ) {
    ASSERT( expr.size_in_bits() == len, "..." );
    Expr res = setval( var.expr(), expr, beg );
    if ( set_scope )
        set_scope->set( var, Var( var.type, res ), Expr(), 0 );
    else
        var.data->ptr->set( res );
}

void RefSliceUnk::write_to_stream( Stream &os ) const {
    os << "slice(" << var << "," << beg << ",len=" << len << ")";
}

#include "../Inst/Slice.h"
#include "RefItem_SetVal.h"
#include "RefItem_Slice.h"
#include "Ip.h"

RefItem_Slice::RefItem_Slice( const Var &var, int beg, int end ) : var( var ), beg( beg ), end( end ) {
}

void RefItem_Slice::write_to_stream( Stream &os ) const {
    os << "slice[" << beg << "," << end << "](" << var << ")";
}

RefItem *RefItem_Slice::shallow_copy() const {
    return new RefItem_Slice( Var( new Ref( var.type(), var.ref->ref_item->shallow_copy() ), 0 ), beg, end );
}

bool RefItem_Slice::contains_a_ptr() const {
    return var.contains_a_ptr();
}

Var RefItem_Slice::pointed_val() const {
    TODO;
    return Var();
}

Expr RefItem_Slice::expr() const {
    return slice( var.expr(), beg, end );
}

void RefItem_Slice::reassign( Ref &dst, RefItem *src ) {
    PRINT( *src );
    if ( src->expr().size_in_bits() != end - beg )
        return ip->disp_error( "slice( ..., len ) <- data of different len" );
    RefItem *res = new RefItem_SetVal( var.ref->ref_item->shallow_copy(), src->shallow_copy(), beg );
    var.ref->ref_item->reassign( *var.ref, res );
    delete res;
}




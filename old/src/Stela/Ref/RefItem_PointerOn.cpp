#include "RefItem_PointerOn.h"
#include "../Inst/PointerOn.h"

RefItem_PointerOn::RefItem_PointerOn( const Var &var ) : var( var ) {
}

void RefItem_PointerOn::write_to_stream( Stream &os ) const {
    os << "&" << var;
}

RefItem *RefItem_PointerOn::shallow_copy() const {
    return new RefItem_PointerOn( var );
}

bool RefItem_PointerOn::contains_a_ptr() const {
    return true;
}


Var RefItem_PointerOn::pointed_val() const {
    return var;
}

Expr RefItem_PointerOn::expr() const {
    return pointer_on( var.expr() );
}

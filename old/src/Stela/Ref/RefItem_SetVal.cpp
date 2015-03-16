#include "../Inst/SetVal.h"
#include "RefItem_SetVal.h"
#include "Var.h"

RefItem_SetVal::RefItem_SetVal( RefItem *old, RefItem *src, int off ) : old( old ), src( src ), off( off ) {
}

RefItem_SetVal::~RefItem_SetVal() {
    delete old;
    delete src;
}

void RefItem_SetVal::write_to_stream( Stream &os ) const {
    os << "set_val(" << *old << "," << *src << ",off=" << off << ")";
}

RefItem *RefItem_SetVal::shallow_copy() const {
    return new RefItem_SetVal( old->shallow_copy(), src->shallow_copy(), off );
}

bool RefItem_SetVal::contains_a_ptr() const {
    return old->contains_a_ptr() or src->contains_a_ptr();
}

Var RefItem_SetVal::pointed_val() const {
    TODO;
    return Var();
}

Expr RefItem_SetVal::expr() const {
    return setval( old->expr(), src->expr(), off );
}


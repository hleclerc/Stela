#include "../System/Assert.h"
#include "RefItem.h"
#include "Ref.h"
#include "Var.h"
#include "Ip.h"

RefItem::RefItem() {
}

RefItem::~RefItem() {
}

void RefItem::write_to_stream( Stream &os ) const {
    os << "RefItem";
}

bool RefItem::contains_a_ptr() const {
    return false;
}

Var RefItem::pointed_val() const {
    return ip->make_error( "not a pointer variable" );
}

void RefItem::reassign( Ref &dst, RefItem *src ) {
    RefItem *old = dst.ref_item;
    dst.ref_item = src->shallow_copy();
    delete old;
}


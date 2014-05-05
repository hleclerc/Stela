#include "RefItem.h"

RefItem::RefItem() {
}

RefItem::~RefItem() {
}

void RefItem::write_to_stream( Stream &os ) const {
    os << "RefItem";
}

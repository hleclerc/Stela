#include "RefItem_Expr.h"

RefItem_Expr::RefItem_Expr( const Expr &expr ) : _expr( expr ) {
}

void RefItem_Expr::write_to_stream( Stream &os ) const {
    os << _expr;
}

RefItem *RefItem_Expr::shallow_copy() const {
    return new RefItem_Expr( _expr );
}

Expr RefItem_Expr::expr() const {
    return _expr;
}

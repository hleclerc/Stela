#include "RefItem_Expr.h"

RefItem_Expr::RefItem_Expr( const Expr &expr ) : expr( expr ) {
}

void RefItem_Expr::write_to_stream( Stream &os ) const {
    os << expr;
}

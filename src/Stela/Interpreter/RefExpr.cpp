#include "RefExpr.h"

RefExpr::RefExpr( const Expr &expr ) : expr( expr ) {
}

Expr RefExpr::get() const {
    return expr;
}

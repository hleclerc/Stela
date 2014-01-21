#include "RefExpr.h"

RefExpr::RefExpr( const Expr &expr ) : expr( expr ) {
}

Expr RefExpr::get() const {
    return expr;
}

void RefExpr::set( Expr expr ) {
    this->expr = expr;
}

void RefExpr::write_to_stream( Stream &os ) const {
    os << expr;
}

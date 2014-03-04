#include "RefExpr.h"

RefExpr::RefExpr( const Expr &data ) : data( data ) {
}

Expr RefExpr::expr() const {
    return data;
}

void RefExpr::set( Expr expr ) {
    data = expr;
}

void RefExpr::write_to_stream( Stream &os ) const {
    os << data;
}

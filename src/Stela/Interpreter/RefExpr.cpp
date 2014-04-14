#include "RefExpr.h"
#include "Scope.h"

RefExpr::RefExpr( const Expr &data ) : data( data ) {
}

Expr RefExpr::expr() const {
    return data;
}

void RefExpr::write_to_stream( Stream &os ) const {
    os << data;
}

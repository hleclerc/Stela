#ifndef EXPR_CST_H
#define EXPR_CST_H

#include "Expr.h"

/// not thread safe
Expr cst( const PI8 *ptr = 0, const PI8 *kno = 0, int size_in_bits = 0 );


/// helper version
template<class T>
Expr cst( const T &value ) {
    return cst( reinterpret_cast<const PI8 *>( &value ), 0, sizeof( value ) * 8 );
}

inline Expr cst( Bool value ) {
    return cst( reinterpret_cast<const PI8 *>( &value ), 0, 1 );
}

#endif // EXPR_CST_H

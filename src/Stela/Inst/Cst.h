#ifndef EXPR_CST_H
#define EXPR_CST_H

#include <string.h>
#include "Expr.h"

/// not thread safe
Expr cst( const Vec<PI8> &value, const Vec<PI8> &known );

///
Expr cst( const Vec<PI8> &value );

/// helper version
template<class T>
Expr cst( const T &value ) {
    Vec<PI8> vec( Size(), sizeof( T ) );
    memcpy( vec.ptr(), &value, sizeof( T ) );
    return cst( vec );
}

#endif // EXPR_CST_H

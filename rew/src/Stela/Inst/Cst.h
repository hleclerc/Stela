#ifndef CST_H
#define CST_H

#include "Inst.h"

Expr cst( int len, const void *data, const void *kwnw = 0 );

// helpers
template<class T>
Expr cst( const T &val ) {
    return cst( 8 * sizeof( val ), &val );
}

Expr cst( bool val );

#endif // CST_H

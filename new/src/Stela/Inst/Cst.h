#ifndef CST_H
#define CST_H

#include "Inst.h"

Ptr<Inst> cst( int size, const void *data, const void *knwn = 0 );

// helpers
template<class T>
Ptr<Inst> cst( const T &value ) {
    return cst( sizeof( value ) * 8, &value );
}

inline Ptr<Inst> cst( Bool value ) {
    return cst( 1, &value );
}

#endif // CST_H

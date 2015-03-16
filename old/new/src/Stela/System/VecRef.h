#ifndef VECREF_H
#define VECREF_H

#include "TypeConfig.h"

/**
*/
template<class T>
class VecRef {
public:
    VecRef( T *beg, ST size ) : _beg( beg ), _size( size ) {}

    int size() const { return _size; }

    const T &operator[]( ST i ) const { return _beg[ i ]; }
    T &operator[]( ST i ) { return _beg[ i ]; }

protected:
    T *_beg;
    ST _size;
};

#endif // VECREF_H

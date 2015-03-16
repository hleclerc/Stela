#ifndef SIZEINBITS_H
#define SIZEINBITS_H

#include "../System/TypeConfig.h"

template<class T>
struct SizeInBits {
    enum { res = 8 * sizeof( T ) };
};

template<>
struct SizeInBits<Bool> {
    enum { res = 1 };
};

#endif // SIZEINBITS_H

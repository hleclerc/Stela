#ifndef SIZEOFIF_H
#define SIZEOFIF_H

template<class T,bool cond>
struct SizeofIf {
    enum { val = 0 };
};

template<class T>
struct SizeofIf<T,true> {
    enum { val = 8 * sizeof( T ) };
};

template<>
struct SizeofIf<bool,true> {
    enum { val = 1 };
};

#endif // SIZEOFIF_H

#ifndef SIZEOFIF_H
#define SIZEOFIF_H

template<class T,bool cond>
struct SizeofIf {
    enum { val = 0 };
};
template<class T>
struct SizeofIf<T,true> {
    enum { val = sizeof( T ) };
};

#endif // SIZEOFIF_H

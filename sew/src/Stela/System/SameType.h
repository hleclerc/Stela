#ifndef SAMETYPE_H
#define SAMETYPE_H

template<class T0,class T1>
struct SameType {
    enum { res = 0 };
};

template<class T0>
struct SameType<T0,T0> {
    enum { res = 1 };
};

#endif // SAMETYPE_H

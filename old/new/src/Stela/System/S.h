#ifndef S_H
#define S_H

/** Used to store a type as an instance... :) e.g. to pass type a parameter of functions */
template<class _T>
struct S {
    typedef _T T;
};

#endif // S_H

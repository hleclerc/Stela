#ifndef N_H
#define N_H

/**
*/
template<int n>
struct N {
    enum { val = n };
    operator int() const { return n; }
};

#endif // N_H

#ifndef RCAST_H
#define RCAST_H

template<class T>
struct RCast {
    RCast( T p ) : p( p ) {
    }
    template<class O> operator O() const {
        return reinterpret_cast<O>( p );
    }
    T p;
};

template<class T>
RCast<T> rcast( T p ) {
    return p;
}

#endif // RCAST_H

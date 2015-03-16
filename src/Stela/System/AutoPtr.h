#ifndef AUTOPTR_H
#define AUTOPTR_H

#include "../System/Assert.h"
#include <stdlib.h>

struct Free {
    template<class T>
    void operator()( T *data ) { if ( data ) free( data ); }
};

struct DeleteArray {
    template<class T>
    void operator()( T *data ) { delete [] data; }
};

struct Delete {
    template<class T>
    void operator()( T *data ) { delete data; }
};

template<class T,class M=Delete>
struct AutoPtr {
    AutoPtr() : data( 0 ) {}
    AutoPtr( T *obj ) : data( obj ) {}
    // AutoPtr( const AutoPtr &obj ) { ERROR( "forbidden" ); }
    AutoPtr( AutoPtr &&obj ) : data( obj.data ) { obj.data = 0; }

    template<class U>
    AutoPtr( const AutoPtr<U> &obj ) { ERROR( "forbidden" ); }

    ~AutoPtr() {
        free_method( data );
    }

    AutoPtr &operator=( T *obj ) {
        if ( data )
            free_method( data );
        data = obj;
        return *this;
    }

    AutoPtr &operator=( const AutoPtr &obj ) {
        if ( data )
            free_method( data );
        data = obj.data;
        obj.data = 0;
        return *this;
    }

    template<class U>
    AutoPtr &operator=( const AutoPtr<U> &obj ) {
        ERROR( "forbidden" );
        return *this;
    }

    operator bool() const { return data; }

    bool operator==( const T           *p ) const { return data == p;      }
    bool operator==( const AutoPtr<T>  &p ) const { return data == p.data; }

    const T *ptr() const { return data; }
    T *ptr() { return data; }

    const T *operator->() const { return data; }
    T *operator->() { return data; }
    const T &operator*() const { return *data; }
    T &operator*() { return *data; }

    template<class Os>
    void write_to_stream( Os &os ) const { if ( data ) os << data; else os << "NULL"; }

    mutable T *data;
    M  free_method;
};

#endif // AUTOPTR_H

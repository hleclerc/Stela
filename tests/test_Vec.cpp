#include <Stela/System/Stream.h>
#include <Stela/System/Vec.h>

struct A {
    A() {
        PRINT( "constroy" );
    }
    A( A && ) {
        PRINT( "move" );
    }
    ~A() {
        PRINT( "destroy" );
    }
};

template<typename Ta,typename Tb>
auto mean( const Ta &a, const Tb &b ) -> decltype( ( a + b ) / 2 ) {
    return ( a + b ) / 2;
}


int main( ) {
    //    Vec<A> v;
    //    v.resize( 1 );
    //    PRINT( __LINE__ );
    //    v.resize( 2 );
    //    PRINT( __LINE__ );
    PRINT( mean( 1, 2.0 ) );
}


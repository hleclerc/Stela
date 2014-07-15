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

int main( ) {
    Vec<A> v;
    v.resize( 1 );
    PRINT( __LINE__ );
    v.resize( 2 );
    PRINT( __LINE__ );
}


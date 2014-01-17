#include "InstVisitor.h"
#include "Rand.h"

int Rand::size_in_bits( int nout ) const {
    return 64;
}

void Rand::write_to_stream( Stream &os ) const {
    os << "rand";
}

void Rand::apply( InstVisitor &visitor ) const {
    visitor( *this );
}

Expr rand_var() {
    return Expr( new Rand, 0 );
}

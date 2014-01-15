#include "Syscall.h"

void Syscall::write_to_stream( Stream &os ) const {
    os << "syscall";
}

syscall::syscall( Expr sys, Expr *inp, int ninp ) {
    Syscall *res = new Syscall;
    res->inp.resize( 1 + ninp );
    res->inp[ 0 ] = sys;
    for( int i = 0; i < ninp; ++i )
        res->inp[ 1 + i ] = inp[ i ];
    // TODO: find if exists somewhere

    this->sys = Expr( res, 0 );
    this->ret = Expr( res, 1 );
}

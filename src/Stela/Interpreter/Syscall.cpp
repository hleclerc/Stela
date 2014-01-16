#include "InstVisitor.h"
#include "Syscall.h"

void Syscall::write_to_stream( Stream &os ) const {
    os << "syscall";
}

void Syscall::apply( InstVisitor &visitor ) const {
    visitor( *this );
}

syscall::syscall( Expr sys, Expr *inp, int ninp ) {
    Syscall *res = new Syscall;
    res->inp_resize( 1 + ninp );
    res->inp_repl( 0, sys );
    for( int i = 0; i < ninp; ++i )
        res->inp_repl( 1 + i, inp[ i ] );

    // TODO: find if exists somewhere

    // ret
    this->sys = Expr( res, 0 );
    this->ret = Expr( res, 1 );
}

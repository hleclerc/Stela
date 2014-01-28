#include "InstVisitor.h"
#include "Syscall.h"
#include "Arch.h"

class Syscall : public Inst_<2,-1> {
public:
    virtual int size_in_bits( int nout ) const { return arch->ptr_size; }
    virtual void write_to_stream( Stream &os ) const { os << "syscall"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.syscall( *this ); }
    virtual int inst_id() const { return Inst::Id_Syscall; }
};


syscall::syscall( Expr sys, int ninp, Expr *inp ) {
    Syscall *res = new Syscall;
    res->inp_resize( 1 + ninp );
    res->inp_repl( 0, sys );
    for( int i = 0; i < ninp; ++i ) {
        ASSERT( inp[ i ].size_in_bits() == arch->ptr_size, "non compatible input for syscall" );
        res->inp_repl( 1 + i, inp[ i ] );
    }

    // ret
    Inst *s = Inst::factorized( res );
    this->sys = Expr( s, 0 );
    this->ret = Expr( s, 1 );
}


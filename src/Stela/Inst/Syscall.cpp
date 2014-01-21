#include "InstVisitor.h"
#include "Syscall.h"

class Syscall : public Inst_<2,-1> {
public:
    virtual int size_in_bits( int nout ) const { return ptr_size; }
    virtual void write_to_stream( Stream &os ) const { os << "syscall"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.syscall( *this, ptr_size ); }
    virtual int inst_id() const { return 2; }
    int ptr_size; ///< size in bits of input and output values
};


syscall::syscall( Expr sys, int ninp, Expr *inp, int ptr_size ) {
    Syscall *res = new Syscall;
    res->inp_resize( 1 + ninp );
    res->inp_repl( 0, sys );
    for( int i = 0; i < ninp; ++i ) {
        ASSERT( inp[ i ].size_in_bits() == ptr_size, "non compatible input for syscall" );
        res->inp_repl( 1 + i, inp[ i ] );
    }
    res->ptr_size = ptr_size;

    // ret
    Inst *s = Inst::factorized( res );
    this->sys = Expr( s, 0 );
    this->ret = Expr( s, 1 );
}


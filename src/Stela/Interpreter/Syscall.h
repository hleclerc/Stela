#ifndef SYSCALL_H
#define SYSCALL_H

#include "Inst_.h"

/**
  out
     -> new sys state
     -> return value
*/
class Syscall : public Inst_<2,-1> {
public:
    virtual int size_in_bits( int nout ) const;
    virtual void write_to_stream( Stream &os ) const;
    virtual void apply( InstVisitor &visitor ) const;
    int bsize; ///< size in bits of input and output values
};

struct syscall {
    syscall( Expr sys, Expr *inp, int ninp, int bsize );
    Expr sys;
    Expr ret;
};

#endif // SYSCALL_H

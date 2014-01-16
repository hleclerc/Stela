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
    virtual void write_to_stream( Stream &os ) const;
    virtual void apply( InstVisitor &visitor ) const;
};

struct syscall {
    syscall( Expr sys, Expr *inp, int ninp );
    Expr sys;
    Expr ret;
};

#endif // SYSCALL_H

#ifndef SYSCALL_H
#define SYSCALL_H

#include "Inst_.h"

/**
  out
     -> new sys state
     -> return value
*/
struct syscall {
    syscall( Expr sys, int ninp, Expr *inp, int ptr_size );
    Expr sys;
    Expr ret;
};

#endif // SYSCALL_H

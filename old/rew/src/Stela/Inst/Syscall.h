#ifndef SYSCALL_H
#define SYSCALL_H

#include "Inst.h"

Expr syscall( const Vec<Expr> &inp, Expr state );

#endif // SYSCALL_H

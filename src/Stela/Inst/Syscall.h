#ifndef STELA_INST_Syscall_H
#define STELA_INST_Syscall_H

#include "Inst.h"

Expr syscall( Vec<Expr> inp, const BoolOpSeq &cond );

#endif // STELA_INST_Syscall_H


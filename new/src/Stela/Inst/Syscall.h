#ifndef SYSCALL_H
#define SYSCALL_H

#include "Inst.h"

/// "raw" syscall
Ptr<Inst> syscall( const Vec<Ptr<Inst> > &children, const Ptr<Inst> &cnd );

#endif // SYSCALL_H

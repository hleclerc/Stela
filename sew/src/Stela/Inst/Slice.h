#ifndef STELA_INST_Slice_H
#define STELA_INST_Slice_H

#include "Inst.h"

Expr slice( Type *dst, Expr var, Expr off );
Expr rcast( Type *dst, Expr var );

#endif // STELA_INST_Slice_H


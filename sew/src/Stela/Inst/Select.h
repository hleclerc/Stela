#ifndef STELA_INST_Select_H
#define STELA_INST_Select_H

#include "Inst.h"

Expr select( const BoolOpSeq &cond, Expr ok, Expr ko );

#endif // STELA_INST_Select_H


#ifndef WHILE_H
#define WHILE_H

#include "Inst.h"

Inst *while_inst( const Vec<Expr> &inp_exprs, const Inst *winp, const Inst *wout, const Vec<int> &corr_inp );
Inst *while_inp( const Vec<int> &sizes_in_bits );
Inst *while_out( const Vec<Expr> &out_exprs );

#endif // WHILE_H

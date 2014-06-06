#ifndef WHILE_H
#define WHILE_H

#include "Inst.h"

Expr while_inst( const Vec<Expr> &inp_exprs, Expr winp, Expr wout, const Vec<int> &corr );
Expr while_inp( const Vec<int> &inp_sizes );
Expr while_out( const Vec<Expr> &out_exprs );

#endif // WHILE_H

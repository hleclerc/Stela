#ifndef STELA_INST_While_H
#define STELA_INST_While_H

#include "Inst.h"

Expr while_inp( const Vec<Type *> types );
Expr while_out( const Vec<Expr> &inp, const Vec<Vec<bool> > &pos );
Expr while_inst( const Vec<Expr> &inp, Expr winp, Expr wout, const Vec<int> &corr );

#endif // STELA_INST_While_H


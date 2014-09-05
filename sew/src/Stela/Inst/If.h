#ifndef IF_H
#define IF_H

#include "Inst.h"

Expr if_inp( const Vec<Type *> &types );
Expr if_out( const Vec<Expr> &inp );
Expr if_inst( const Vec<Expr> &inp, Expr if_inp_ok, Expr if_inp_ko, Expr if_out_ok, Expr if_out_ko );

#endif // IF_H

#ifndef SETVAL_H
#define SETVAL_H

#include "Expr.h"

/// replace bits [ beg, beg + b.size ] of a with bits of b
Expr setval( Expr a, Expr b, Expr beg, bool beg_in_bits = true );

/// replace bits [ beg, beg + b.size ] of a with bits of b
Expr setval( Expr a, Expr b, int beg, bool beg_in_bits = true );

#endif // SETVAL_H

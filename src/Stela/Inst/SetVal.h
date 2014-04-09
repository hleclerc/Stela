#ifndef SETVAL_H
#define SETVAL_H

#include "Expr.h"

/// replace bits [ beg, beg + b.size ] of a with bits of b
Expr setval( Expr a, Expr b, Expr beg );

#endif // SETVAL_H

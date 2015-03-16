#ifndef Expr_SLICE_H
#define Expr_SLICE_H

#include "Expr.h"

/// return [beg;beg+len) bits from slice
Expr slice( Expr expr, const Expr &beg, int len );

/// return [beg;end) bits from slice (known beg and end version)
Expr slice( Expr expr, int beg, int end );


#endif // Expr_SLICE_H

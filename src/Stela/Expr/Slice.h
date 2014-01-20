#ifndef Expr_SLICE_H
#define Expr_SLICE_H

#include "Expr.h"

namespace Expr_NS {


/// return an bit slice ([beg;end[) of an expression
Expr slice( Expr expr, int beg, int end );

}


#endif // Expr_SLICE_H

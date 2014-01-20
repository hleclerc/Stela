#ifndef Expr_Reassign_H
#define Expr_Reassign_H

#include "Expr.h"

namespace Expr_NS {


/// return an expression with values equal to expr, expected
/// for bits [off;off+val.size_in_bits()[ where it is equal to val
Expr reassign( Expr expr, Expr val, int off );

}

#endif // Expr_Reassign_H

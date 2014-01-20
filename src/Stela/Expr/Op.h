#ifndef Expr_OP_H
#define Expr_OP_H

#include "BaseType.h"
#include "Expr.h"


namespace Expr_NS {

#define DECL_OP( OP ) Expr OP( const BaseType *bt, Expr a, Expr b );
#include "DeclOpBinary.h"
#undef DECL_OP

#define DECL_OP( OP ) Expr OP( const BaseType *bt, Expr a );
#include "DeclOpUnary.h"
#undef DECL_OP

}

#endif // Expr_OP_H

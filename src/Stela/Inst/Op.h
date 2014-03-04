#ifndef Expr_OP_H
#define Expr_OP_H

#include "OpStructs.h"
#include "BaseType.h"
#include "Expr.h"


// e.g. op_add( bt_SI32, a, b )
#define DECL_IR_TOK( OP ) Expr op_##OP( const BaseType *bt, Expr a, Expr b );
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( OP ) Expr op_##OP( const BaseType *bt, Expr a );
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK


// e.g. op( bt_SI32, a, b, Op_Add() )
#define DECL_IR_TOK( OP ) Expr op( const BaseType *bt, Expr a, Expr b, Op_##OP );
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( OP ) Expr op( const BaseType *bt, Expr a, Op_##OP );
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK


#endif // Expr_OP_H

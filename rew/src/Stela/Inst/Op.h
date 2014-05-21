#ifndef OP_H
#define OP_H

#include "Inst.h"
class Type;

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    struct Op_##NAME { void write_to_stream( Stream &os ) const { os << #NAME; } enum { is_oper = OPER, n = 2, prec = PREC }; void write_oper( Stream &os ) const { os << GEN; } }; \
    Expr op( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_##NAME ); \
    enum { PREC_##NAME = PREC };
#include "DeclOp_Binary.h"
#undef DECL_OP

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    struct Op_##NAME { void write_to_stream( Stream &os ) const { os << #NAME; } enum { is_oper = OPER, n = 1, prec = PREC }; void write_oper( Stream &os ) const { os << GEN; } }; \
    Expr op( Type *tr, Type *ta, Expr a, Op_##NAME ); \
    enum { PREC_##NAME = PREC };
#include "DeclOp_Unary.h"
#undef DECL_OP

#endif // OP_H

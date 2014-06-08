#ifndef STELA_INST_Op_H
#define STELA_INST_Op_H

#include "Inst.h"

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    struct Op_##NAME { void write_to_stream( Stream &os ) const { os << #NAME; } enum { is_oper = OPER, n = 2, prec = PREC, b = BOOL }; void write_oper( Stream &os ) const { os << #GEN; } }; \
    Expr op( Expr a, Expr b, Op_##NAME ); \
    Expr NAME( Expr a, Expr b ); \
    enum { PREC_##NAME = PREC };
#include "DeclOp_Binary.h"
#undef DECL_OP

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    struct Op_##NAME { void write_to_stream( Stream &os ) const { os << #NAME; } enum { is_oper = OPER, n = 1, prec = PREC, b = BOOL }; void write_oper( Stream &os ) const { os << #GEN; } }; \
    Expr op( Expr a, Op_##NAME ); \
    Expr NAME( Expr a ); \
    enum { PREC_##NAME = PREC };
#include "DeclOp_Unary.h"
#undef DECL_OP

#endif // STELA_INST_Op_H


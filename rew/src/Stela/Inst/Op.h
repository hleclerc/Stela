#ifndef OP_H
#define OP_H

#include "Inst.h"
class Type;

#define DECL_OP( NAME, GEN, OPER, BOOL ) \
    struct Op_##NAME { void write_to_stream( Stream &os ) const { os << #NAME; } }; \
    Expr op( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_##NAME );
#include "DeclOp_Binary.h"
#undef DECL_OP

#endif // OP_H

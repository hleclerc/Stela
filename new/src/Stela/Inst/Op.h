#ifndef OP_H
#define OP_H

#include "Inst.h"
class Type;

#define DECL_OP( NAME, GEN, OPER, BOOL ) \
    struct Op_##NAME { void write_to_stream( Stream &os ) const { os << #NAME; } }; \
    Ptr<Inst> op( Type *tr, Type *ta, Ptr<Inst> a, Type *tb, Ptr<Inst> b, Op_##NAME );
#include "DeclOp_Binary.h"
#undef DECL_OP

#endif // OP_H

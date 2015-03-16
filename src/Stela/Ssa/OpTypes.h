#ifndef OPTYPES_H
#define OPTYPES_H

enum {
    #define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) ID_OP_##NAME,
    #include "DeclOp_Binary.h"
    #include "DeclOp_Unary.h"
    #undef DECL_OP

    ID_OP_ReplBits,
    ID_OP_Room,
    ID_OP_Slice,
    ID_OP_WhileInp,
    ID_OP_WhileOut,
    ID_OP_WhileInst,
    ID_OP_IfInp,
    ID_OP_IfOut,
    ID_OP_IfInst,
    ID_OP_Store,
    ID_OP_Uninitialized,
    ID_OP_UnknownInst,
    ID_OP_GetNout,
    ID_OP_Copy,
    ID_OP_Conv,
    ID_OP_Select,
    ID_OP_Cst,
    ID_OP_Syscall,
    ID_OP_Symbol,
    ID_OP_PointedValue,
    ID_OP_Rcast,
    ID_OP_CstComputedSize,
    ID_OP_InstBlock
};



#endif // OPTYPES_H

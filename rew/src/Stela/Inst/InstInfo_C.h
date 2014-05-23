#ifndef INSTINFO_C_H
#define INSTINFO_C_H

#include "Codegen_C.h"
#include "Inst.h"
class CInstBlock;
class OutReg;

/**
*/
class InstInfo_C {
public:
    InstInfo_C();

    CInstBlock *block;

    int num_reg;
    Type *out_type;
    Type *val_type; ///< for pointers
    OutReg *out_reg;
};

#define IIC( A ) reinterpret_cast<InstInfo_C *>( ( A )->op_mp )

#endif // INSTINFO_C_H

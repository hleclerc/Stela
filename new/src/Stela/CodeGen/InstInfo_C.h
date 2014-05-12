#ifndef INSTINFO_C_H
#define INSTINFO_C_H

#include "CodeGen_C.h"
class Type;

/**
*/
class InstInfo_C {
public:
    InstInfo_C();

    int num_reg;
    const Inst *sched_in;
};

#define IIC( A ) reinterpret_cast<InstInfo_C *>( ( A )->op_mp )

#endif // INSTINFO_C_H

#ifndef INSTINFO_C_H
#define INSTINFO_C_H

#include "Inst.h"
class InstBlock;

/**
*/
class InstInfo_C {
public:
    InstInfo_C( Expr when );

    InstBlock *block;
    Expr when;
};

#define IIC( A ) reinterpret_cast<InstInfo_C *>( ( A )->op_mp )

#endif // INSTINFO_C_H

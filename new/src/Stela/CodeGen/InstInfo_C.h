#ifndef INSTINFO_C_H
#define INSTINFO_C_H

#include "CodeGen_C.h"
class Type;

/**
*/
class InstInfo_C {
public:
    InstInfo_C( const Ptr<Inst> &inst_false );
    bool add_when_possibility( const Ptr<Inst> &cond ); ///< true if already done in included in current when

    int num_reg;
    Ptr<Inst> when; ///< conditions for inst to be executed
    const Inst *sched_in;
};

#define IIC( A ) reinterpret_cast<InstInfo_C *>( ( A )->op_mp )

#endif // INSTINFO_C_H

#ifndef CPPOUTREG_H
#define CPPOUTREG_H

#include "../Inst/Inst.h"

/**
*/
class CppOutReg {
public:
    CppOutReg( Type *type, int num );
    Stream &write( Codegen_C *cc, bool new_reg );
    Type *type;
    int num;
};

#endif // CPPOUTREG_H

#ifndef CPPOUTREG_H
#define CPPOUTREG_H

#include "../Inst/Inst.h"

/**
*/
class CppOutReg {
public:
    CppOutReg( Type *type, int num );

    void write_to_stream( Stream &os ) const;

    Stream &write( Codegen_C *cc, bool new_reg );
    Inst *common_provenance_ancestor();

    Vec<Inst *> provenance; ///< where it is used
    Type *type;
    int num;
};

#endif // CPPOUTREG_H

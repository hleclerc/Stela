#ifndef CPPOUTREG_H
#define CPPOUTREG_H

#include "../Inst/Inst.h"
struct CC_SeqItemExpr;

/**
*/
class CppOutReg {
public:
    CppOutReg( Type *type, int num );

    void write_to_stream( Stream &os ) const;

    Stream &write( Codegen_C *cc, bool new_reg );
    CC_SeqItemBlock *common_provenance_ancestor();

    Vec<CC_SeqItemExpr *> provenance;
    Type *type;
    int num;
};

#endif // CPPOUTREG_H

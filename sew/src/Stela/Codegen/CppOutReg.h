#ifndef CPPOUTREG_H
#define CPPOUTREG_H

#include "../Inst/Inst.h"
struct CC_SeqItemBlock;

/**
*/
class CppOutReg {
public:
    CppOutReg( Type *type, int num, CC_SeqItemBlock *parent_block );

    Stream &write( Codegen_C *cc, bool new_reg );
    CC_SeqItemBlock *common_provenance_ancestor();

    Vec<CC_SeqItemBlock *> provenance;
    Type *type;
    int num;
};

#endif // CPPOUTREG_H

#ifndef INSTBLOCK_H
#define INSTBLOCK_H

#include "../Ssa/Inst.h"

/**
*/
class InstBlock {
public:
    InstBlock( InstBlock *parent );

    void operator<<( Inst *inst );

    InstBlock *parent;
    Inst *beg;
    Inst *end;
};

#endif // INSTBLOCK_H

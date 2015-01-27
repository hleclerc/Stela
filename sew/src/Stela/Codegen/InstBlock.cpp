#include "InstBlock.h"

InstBlock::InstBlock( InstBlock *parent ) : parent( parent ) {
    beg = 0;
    end = 0;
}

void InstBlock::operator<<( Inst *inst ) {
    if ( beg ) {
        end->next_sched = inst;
        inst->prev_sched = end;
    } else
        beg = inst;
    end = inst;
}

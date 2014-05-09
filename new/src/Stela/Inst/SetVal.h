#ifndef SETVAL_H
#define SETVAL_H

#include "Inst.h"

Ptr<Inst> set_val( Ptr<Inst> src, Ptr<Inst> val, Vec<Ptr<Inst> > conds, int offset );

#endif // SETVAL_H

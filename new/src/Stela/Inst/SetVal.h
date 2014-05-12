#ifndef SETVAL_H
#define SETVAL_H

#include "Inst.h"

Ptr<Inst> set_val( Ptr<Inst> src, Ptr<Inst> val, const Vec<Ptr<Inst> > &cnd, int offset );

#endif // SETVAL_H

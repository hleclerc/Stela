#ifndef OP_H
#define OP_H

#include "Inst.h"
class Type;

struct Op_add { void write_to_stream( Stream &os ) const { os << "add"; } };

Ptr<Inst> op( Type *tr, Type *ta, Ptr<Inst> a, Type *tb, Ptr<Inst> b, Op_add );

#endif // OP_H

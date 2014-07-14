#ifndef CPPGETCONSTRAINT_H
#define CPPGETCONSTRAINT_H

#include "../Inst/Inst.h"
#include <map>
class CC_SeqItemExpr;

///< helpers for output liveness
struct CppGetConstraint {
    std::map<Inst *,int> cur_live_outputs;
    Vec<CC_SeqItemExpr *> seq;
};


#endif // CPPGETCONSTRAINT_H

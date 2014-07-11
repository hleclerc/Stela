#ifndef CPPGETCONSTRAINT_H
#define CPPGETCONSTRAINT_H

#include "../Inst/Inst.h"
#include <map>

///< helpers for output liveness
struct CppGetConstraint {
    std::map<Inst *,int> cur_live_outputs;
    Vec<Inst *> seq;
};


#endif // CPPGETCONSTRAINT_H

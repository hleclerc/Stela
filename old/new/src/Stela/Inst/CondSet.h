#ifndef CONDSET_H
#define CONDSET_H

#include "../System/Vec.h"
#include "../System/Ptr.h"
class Inst;

/**
*/
class CondSet {
public:
    CondSet();



    Vec<Ptr<Inst> > inst_vec;
};

#endif // CONDSET_H

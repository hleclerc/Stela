#ifndef INST_H
#define INST_H

#include "../System/Ptr.h"
#include "../System/Vec.h"

/**
*/
class Inst : public ObjectWithCptUse {
public:
    Inst();
    virtual ~Inst();

    virtual const PI8 *cst_data( int nout ) const;
};

#endif // INST_H

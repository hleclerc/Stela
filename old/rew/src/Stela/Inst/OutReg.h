#ifndef OUTREG_H
#define OUTREG_H

#include "Inst.h"

/**
*/
class OutReg {
public:
    OutReg( Type *type, int num );
    void write_to_stream( Stream &os ) const;

    Type *type;
    int num;
};

#endif // OUTREG_H

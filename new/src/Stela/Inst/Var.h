#ifndef VAR_H
#define VAR_H

#include "Inst.h"
class Type;

/**
*/
class Var {
public:
    // flags
    enum {
        WEAK_CONST = 1
    };

    Var( Type *type = 0 ); ///< uninialized variable
    Var( SI32 val );

    void write_to_stream( Stream &os ) const;

    Ptr<Inst> inst;
    Type     *type;
    int       flags;
};

#endif // VAR_H

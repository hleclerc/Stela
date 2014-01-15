#ifndef CST_H
#define CST_H

#include "Expr.h"

/**
*/
class Cst : public Inst {
public:
    virtual ~Cst();
    virtual const PI8 *cst_data( int nout ) const;
    virtual void write_to_stream( Stream &os ) const;

    Vec<PI8> value; ///< value (should not be changed directly)
    Vec<PI8> known; ///< known bits
};

/// not thread safe
Expr cst( Vec<PI8> value );

#endif // CST_H

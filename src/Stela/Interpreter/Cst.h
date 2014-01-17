#ifndef CST_H
#define CST_H

#include "Inst_.h"

/**
*/
class Cst : public Inst_<1,0> {
public:
    virtual ~Cst();
    virtual int size_in_bits( int nout ) const;
    virtual const PI8 *cst_data( int nout ) const;
    virtual void write_to_stream( Stream &os ) const;
    virtual void apply( InstVisitor &visitor ) const;

    Vec<PI8> value; ///< value (should not be changed directly)
    Vec<PI8> known; ///< known bits
};

/// not thread safe
Expr cst( Vec<PI8> value );

#endif // CST_H

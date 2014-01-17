#ifndef RAND_H
#define RAND_H

#include "Inst_.h"

/**
*/
class Rand : public Inst_<1,0> {
public:
    virtual void write_to_stream( Stream &os ) const;
    virtual void apply( InstVisitor &visitor ) const;
};

Expr rand_var();

#endif // RAND_H

#ifndef SUBVALUE_H
#define SUBVALUE_H

#include "Inst_.h"

/**

*/
class SubValue : public Inst_<1,2> {
public:
    int off;
};

/// return an expression with values equal to expr, expected
/// for bits [off;off+val.size_in_bits()[ where it is equal to val
Expr sub_value( Expr expr, Expr val, int off );

#endif // SUBVALUE_H

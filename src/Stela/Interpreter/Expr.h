#ifndef EXPR_H
#define EXPR_H

#include "Inst.h"

/**
*/
class Expr {
public:
    Expr( Ptr<Inst> inst, int nout = 0 );
    Expr();

    const PI8 *cst_data() const;
    void write_to_stream( Stream &os ) const;
    operator bool() const { return inst; }

    Vec<Inst::Out::Item,-1,1> &parents() { return inst->out_expr( nout ).parents; }

    Ptr<Inst> inst;
    int       nout; ///< num output of inst to use
};

#endif // EXPR_H

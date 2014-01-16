#include "Expr.h"

PI64 Inst::cur_op_id = 0;

Inst::Inst() {
    ext_parent = 0;

    op_id_viz  = 0;
    op_id      = 0;
    op_mp      = 0;
}

Inst::~Inst() {
}

const PI8 *Inst::cst_data( int nout ) const {
    return 0;
}


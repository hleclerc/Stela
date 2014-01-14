#include "Expr.h"

Expr::Expr( Ptr<Inst> inst, int nout ) : inst( inst ), nout( nout ) {
}

Expr::Expr() {
}

const PI8 *Expr::cst_data() const {
    return inst->cst_data( nout );
}

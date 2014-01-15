#include "Expr.h"

Expr::Expr( Ptr<Inst> inst, int nout ) : inst( inst ), nout( nout ) {
}

Expr::Expr() {
}

const PI8 *Expr::cst_data() const {
    return inst->cst_data( nout );
}

void Expr::write_to_stream( Stream &os ) const {
    os << inst;
    if ( inst->out.size() > 1 )
        os << '(' << nout << ')';
}

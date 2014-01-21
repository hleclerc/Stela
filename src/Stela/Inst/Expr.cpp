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
    if ( inst->out_size() > 1 )
        os << '(' << nout << ')';
}

int Expr::size_in_bits() const {
    return inst->size_in_bits( nout );
}

int Expr::size_in_bytes() const {
    return inst->size_in_bytes( nout );
}

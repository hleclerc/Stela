#include "Expr.h"

Expr::Expr( Ptr<Inst> inst, int nout ) : inst( inst ), nout( nout ) {
}

Expr::Expr() {
}

const PI8 *Expr::cst_data( int beg, int end ) const {
    return inst->cst_data( nout, beg, end );
}

const PI8 *Expr::cst_data() const {
    return cst_data( 0, size_in_bits() );
}

const PI8 *Expr::vat_data( int beg, int end ) const {
    return inst->vat_data( nout, beg, end );
}

const PI8 *Expr::vat_data() const {
    return vat_data( 0, size_in_bits() );
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

const BaseType *Expr::out_bt() const {
    return inst->out_bt( nout );
}


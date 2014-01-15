#include "Expr.h"

Inst::Inst() {
}

Inst::~Inst() {
}

const PI8 *Inst::cst_data( int nout ) const {
    return 0;
}

void Inst::InpList::Setter::operator=( const Expr &expr ) {
    inl->_data[ ind ] = expr;
}

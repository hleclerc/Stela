#include "Inst.h"

// see _Expr_defs.h for inlined methods

void Expr::write_to_stream( Stream &os ) const {
    if ( inst )
        os << *inst;
    else
        os << "NULL";
}

Expr simplified( Expr val ) {
    if ( Expr res = val->_simplified() )
        return res;
    return val;
}


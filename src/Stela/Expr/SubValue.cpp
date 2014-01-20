#include "SubValue.h"

Expr sub_value( Expr expr, Expr val, int off ) {
    SubValue *res = new SubValue;
    res->inp_repl( 0, expr );
    res->inp_repl( 1, val );
    res->off = off;
    return Expr( Inst::factorized( res ), 0 );
}

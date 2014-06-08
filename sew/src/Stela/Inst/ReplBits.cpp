#include "ReplBits.h"
#include "Ip.h"

/**
*/
struct ReplBits : Inst {
    virtual void write_dot( Stream &os ) { os << "ReplBits"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new ReplBits; }
    virtual Type *type() { return inp[ 0 ]->type(); }
};

Expr repl_bits( Expr src, Expr off, Expr val ) {
    SI32 voff;
    PRINT( off->get_val( ip->type_SI32, &voff ) );
    PRINT( voff );
    if ( off->get_val( ip->type_SI32, &voff ) and voff == 0 and src->size() == val->size() )
        return val;
    if ( Expr res = src->_simp_repl_bits( off, val ) )
        return res;

    ReplBits *res = new ReplBits;
    res->add_inp( src );
    res->add_inp( off );
    res->add_inp( val );
    return res;
}


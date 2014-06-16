#include "ReplBits.h"
#include "Slice.h"
#include "Type.h"
#include "Ip.h"
#include "Op.h"

/**
  repl_bits( src, off, val )
*/
struct ReplBits : Inst {
    virtual void write_dot( Stream &os ) { os << "ReplBits"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new ReplBits; }
    virtual Type *type() { return inp[ 0 ]->type(); }
    virtual Expr _simp_slice( Type *dst, Expr off ) {
        SI32 beg, len = inp[ 2 ]->size();
        if ( not inp[ 1 ]->get_val( ip->type_SI32, &beg ) ) return (Inst *)0;
        SI32 nbeg, nlen = dst->size();
        if ( not off->get_val( ip->type_SI32, &nbeg ) ) return (Inst *)0;
        SI32 end = beg + len, nend = nbeg + nlen;
        //
        if ( nend <= beg or nbeg >= end ) return slice( dst, inp[ 0 ], off );
        if ( nbeg >= beg and nend <= end ) return slice( dst, inp[ 2 ], sub( off, inp[ 1 ] ) );

        return Inst::_simp_slice( dst, off );
    }
};

Expr repl_bits( Expr src, Expr off, Expr val ) {
    SI32 voff;
    // replace all the bits (but keep the same type)
    if ( off->get_val( ip->type_SI32, &voff ) and voff == 0 and src->size() == val->size() )
        return rcast( src->type(), val );
    //
    if ( val->size() == 0 )
        return src;
    // possible simplification
    if ( Expr res = src->_simp_repl_bits( off, val ) )
        return res;

    ReplBits *res = new ReplBits;
    res->add_inp( src );
    res->add_inp( off );
    res->add_inp( val );
    return res;
}


#include "ReplBits.h"
#include "Rcast.h"
// #include "Slice.h"
// #include "Type.h"
// #include "Ip.h"
// #include "Op.h"

/**
  repl_bits( src, off, val )
*/
struct ReplBits : Inst {
    virtual void write_dot( Stream &os ) const {
        os << "ReplBits";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new ReplBits;
    }
    virtual Type *type() {
        return inp[ 0 ]->type();
    }
};

Expr repl_bits( Expr src, Expr off, Expr val ) {
    // replace all the bits (but keep the same type)
    //if ( off == 0 and src->size() == val->size() )
    //    return rcast( src->type(), val );
    //
    //if ( val->size() == 0 )
    //    return src;
    // possible simplification
    // if ( Expr res = src->_simp_repl_bits( off, val ) )
    //     return res;

    ReplBits *res = new ReplBits;
    res->add_inp( src );
    res->add_inp( off );
    res->add_inp( val );
    return Inst::twin_or_val( res );
}


#include "GlobalVariables.h"
#include "ReplBits.h"
#include "Rcast.h"
#include "Slice.h"
#include "Type.h"
 #include "Op.h"

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
    virtual int op_type() const { return ID_OP_ReplBits; }
    virtual Type *type() {
        return inp[ 0 ]->type();
    }
    virtual Expr _simp_slice( Type *dst, Expr off ) {
        //           inp[ 1 ]
        //           |          inp[ 1 ] + inp[ 2 ]->size()
        //           |          |
        //           v          v
        // -inp[ 0 ]- -inp[ 2 ]- -inp[ 0 ]-
        //
        //    off
        //    |           off + dst->size()
        //    |           |
        //    v           v
        //    [-----------]
        //
        if ( sup_eq( off, add( inp[ 1 ], inp[ 2 ]->size() ) )->always( true ) )
            return slice( dst, inp[ 0 ], off );

        int s = dst->size();
        if ( s >= 0 and sup_eq( off, inp[ 1 ] )->always( true ) and inf_eq( add( off, s ), add( inp[ 1 ], inp[ 2 ]->size()) )->always( true ) )
            return slice( dst, inp[ 2 ], sub( off, inp[ 1 ] ) );

        if ( s >= 0 and inf_eq( add( off, s ), inp[ 1 ] )->always( true ) )
            return slice( dst, inp[ 0 ], off );

        return Expr();
    }
};

Expr repl_bits( Expr src, Expr off, Expr val ) {
    // replace all the bits (but keep the same type)
    int z = 0;
    if ( off->always_equal( ip->type_SI32, &z ) and equ( src->size(), val->size() )->always( true ) )
        return rcast( src->type(), val );

    // size of val is 0
    if ( val->size()->always_equal( ip->type_SI32, &z ) )
        return src;

    // specific simplification (e.g. for cst)
    if ( Expr res = src->_simp_repl_bits( off, val ) )
        return res;

    ReplBits *res = new ReplBits;
    res->add_inp( src );
    res->add_inp( off );
    res->add_inp( val );
    return Inst::twin_or_val( res );
}


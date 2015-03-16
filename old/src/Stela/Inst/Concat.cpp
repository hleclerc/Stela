#include "../System/Memcpy.h"
#include "InstVisitor.h"
#include "Concat.h"
#include "Slice.h"
#include "Inst_.h"
#include "Cst.h"

///
class Concat : public Inst_<1,2> {
public:
    virtual int size_in_bits( int nout ) const { return inp_expr( 0 ).size_in_bits() + inp_expr( 1 ).size_in_bits(); }
    virtual void write_dot( Stream &os ) const { os << "concat"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.concat( *this ); }
    virtual int inst_id() const { return Inst::Id_Concat; }
    virtual Expr _smp_slice( int nout, int beg, int end ) {
        // basic simplifications
        if ( Expr res = Inst::_smp_slice( nout, beg, end ) )
            return res;
        // specific ones
        int sep = inp_expr( 0 ).size_in_bits();
        if ( end <= sep )
            return inp_expr( 0 ).inst->_smp_slice( inp_expr( 0 ).nout, beg, end );
        if ( beg >= sep )
            return inp_expr( 1 ).inst->_smp_slice( inp_expr( 1 ).nout, beg - sep, end - sep );
        return Expr();
    }
    virtual const PI8 *cst_data( int nout, int beg, int end ) const {
        int sep = inp_expr( 0 ).size_in_bits();
        if ( end <= sep )
            return inp_expr( 0 ).cst_data( beg, end );
        if ( beg >= sep )
            return inp_expr( 1 ).cst_data( beg - sep, end - sep );
        return 0;
    }
};

Expr concat( Expr a, Expr b ) {
    if ( a.size_in_bits() == 0 )
        return b;
    if ( b.size_in_bits() == 0 )
        return a;

    // simplifications ?
    if ( const PI8 *da = a.cst_data() ) {
        if ( const PI8 *db = b.cst_data() ) {
            int p_0 = 0, p_1 = a.size_in_bits(), p_2 = p_1 + b.size_in_bits();
            Vec<PI8> res( Size(), ( p_2 + 7 ) / 8, 0 );
            memcpy_bit( res.ptr(), p_0, da, 0, p_1 - p_0 );
            memcpy_bit( res.ptr(), p_1, db, 0, p_2 - p_1 );
            return cst( res.ptr(), 0, p_2 );
        }
    }

    // else create a new inst
    Concat *res = new Concat;
    res->inp_repl( 0, a );
    res->inp_repl( 1, b );
    return Expr( Inst::factorized( res ), 0 );
}

Expr concat( Expr a, Expr b, Expr c ) {
    return concat( concat( a, b ), c );
}

Expr concat( Expr a, Expr b, Expr c, Expr d ) {
    return concat( concat( concat( a, b ), c ), d );
}

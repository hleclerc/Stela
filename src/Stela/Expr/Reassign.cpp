#include "../System/Memcpy.h"
#include "InstVisitor.h"
#include "Reassign.h"
#include "Slice.h"
#include "Inst_.h"
#include "Cst.h"

namespace Expr_NS {

///
class Reassign : public Inst_<1,2> {
public:
    virtual int size_in_bits( int nout ) const { return inp_expr( 0 ).size_in_bits(); }
    virtual void write_to_stream( Stream &os ) const { os << "reassign"; }
    virtual void apply( InstVisitor &visitor ) const { visitor( *this, off ); }
    virtual int inst_id() const { return 5; }
    virtual bool equal( const Inst *b ) const {
        return Inst::equal( b ) and off == static_cast<const Reassign *>( b )->off;
    }
    int off;
};

Expr reassign( Expr expr, Expr val, int off ) {
    ASSERT( expr.size_in_bits() >= off + val.size_in_bits(), "Wrong size" );

    // simplifications ?
    if ( off == 0 and expr.size_in_bits() == val.size_in_bits() )
        return val;

    int p[] = { 0, off, off + val.size_in_bits(), expr.size_in_bits() };
    Expr sa = slice( expr, p[ 0 ], p[ 1 ] );
    Expr sc = slice( expr, p[ 2 ], p[ 3 ] );
    const PI8 *da = sa.cst_data();
    const PI8 *db = val.cst_data();
    const PI8 *dc = sc.cst_data();
    if ( da and db and dc ) {
        Vec<PI8> res( Size(), expr.size_in_bytes(), 0 );
        memcpy_bit( res.ptr(), p[ 0 ], da, 0, p[ 1 ] - p[ 0 ] );
        memcpy_bit( res.ptr(), p[ 1 ], db, 0, p[ 2 ] - p[ 1 ] );
        memcpy_bit( res.ptr(), p[ 2 ], dc, 0, p[ 3 ] - p[ 2 ] );
        return cst( res );
    }

    // else create a new inst
    Reassign *res = new Reassign;
    res->inp_repl( 0, expr );
    res->inp_repl( 1, val );
    res->off = off;
    return Expr( Inst::factorized( res ), 0 );
}

}

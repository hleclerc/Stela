#include "InstVisitor.h"
#include "Reassign.h"
#include "Inst_.h"
#include "Cst.h"

namespace Expr_NS {

/**

*/
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

    const PI8 *da = expr.cst_data();
    const PI8 *db = val.cst_data();
    if ( da and db ) {
        Vec<PI8> res( Size(), expr.size_in_bytes() );
        memcpy( res.ptr(), da, expr.size_in_bytes() );
        if ( off % 8 or val.size_in_bits() % 8 )
            TODO;
        else
            memcpy( res.ptr() + off / 8, db, val.size_in_bytes() );
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

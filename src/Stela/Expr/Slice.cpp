#include "../System/Memcpy.h"
#include "InstVisitor.h"
#include "Slice.h"
#include "Inst_.h"
#include "Cst.h"

namespace Expr_NS {

///
class Slice : public Inst_<1,1> {
public:
    virtual int size_in_bits( int nout ) const { return end - beg; }
    virtual void write_to_stream( Stream &os ) const { os << "slice"; }
    virtual void apply( InstVisitor &visitor ) const { visitor( *this, beg, end ); }
    virtual int inst_id() const { return 6; }
    virtual bool equal( const Inst *b ) const {
        return Inst::equal( b ) and 
            beg == static_cast<const Slice *>( b )->beg and 
            end == static_cast<const Slice *>( b )->end;
    }
    int beg, end;
};

Expr slice( Expr expr, int beg, int end ) {
    ASSERT( beg <= expr.size_in_bits(), "Wrong size" );
    ASSERT( end <= expr.size_in_bits(), "Wrong size" );
    ASSERT( beg <= end, "Wrong size" );
    ASSERT( beg >= 0, "Wrong size" );
    ASSERT( end >= 0, "Wrong size" );

    // simplifications ?
    if ( beg == 0 and end == expr.size_in_bits() )
        return expr;
    if ( beg == end )
        return cst( Vec<PI8>() );

    if ( const PI8 *da = expr.cst_data() ) {
        Vec<PI8> res( Size(), ( end - beg + 7 ) / 8 );
        memcpy_bit( res.ptr(), 0, da, beg, end - beg );
        return cst( res );
    }

    // else create a new inst
    Slice *res = new Slice;
    res->inp_repl( 0, expr );
    res->beg = beg;
    res->end = end;
    return Expr( Inst::factorized( res ), 0 );
}

}

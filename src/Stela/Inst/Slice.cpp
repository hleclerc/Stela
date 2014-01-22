#include "InstVisitor.h"
#include "Slice.h"
#include "Inst_.h"
#include "Cst.h"

///
class Slice : public Inst_<1,1> {
public:
    virtual int size_in_bits( int nout ) const { return end - beg; }
    virtual void write_to_stream( Stream &os ) const { os << "slice(" << inp_expr( 0 ) << "," << beg << "," << end << ")"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.slice( *this, beg, end ); }
    virtual int inst_id() const { return Inst::Id_Slice; }
    virtual bool equal( const Inst *b ) const {
        return Inst::equal( b ) and 
            beg == static_cast<const Slice *>( b )->beg and 
            end == static_cast<const Slice *>( b )->end;
    }
    int beg, end;
};

Expr slice( Expr expr, int beg, int end ) {
    ASSERT( beg >= 0 and beg <= expr.size_in_bits(), "Wrong size" );
    ASSERT( end >= 0 and end <= expr.size_in_bits(), "Wrong size" );
    ASSERT( beg <= end, "Wrong size" );

    // simplifications ?
    if ( Expr res = expr.inst->_smp_slice( expr.nout, beg, end ) )
        return res;

    // else create a new inst
    Slice *res = new Slice;
    res->inp_repl( 0, expr );
    res->beg = beg;
    res->end = end;
    return Expr( Inst::factorized( res ), 0 );
}

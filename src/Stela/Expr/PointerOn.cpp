#include "InstVisitor.h"
#include "PointerOn.h"
#include "Slice.h"
#include "Inst_.h"

namespace Expr_NS {

/**
*/
class PointerOn : public Inst_<1,1> {
public:
    virtual int size_in_bits( int nout ) const { return ptr_size; }
    virtual void write_to_stream( Stream &os ) const { os << "pointer_on(" << inp_expr( 0 ) << ")"; }
    virtual void apply( InstVisitor &visitor ) const { visitor( *this, ptr_size ); }
    virtual int inst_id() const { return 3; }
    virtual Expr _smp_val_at( int nout, int size ) {
        return slice( inp_expr( 0 ), 0, size );
    }
    int ptr_size;
};


Expr pointer_on( Expr expr, int ptr_size ) {
    // simplifications
    if ( Expr res = expr.inst->_smp_pointer_on( expr.nout ) ) {
        ASSERT( ptr_size == res.size_in_bits(), "..." );
        return res;
    }

    //
    PointerOn *res = new PointerOn;
    res->ptr_size = ptr_size;
    res->inp_repl( 0, expr );
    return Expr( Inst::factorized( res ), 0 );
}

}

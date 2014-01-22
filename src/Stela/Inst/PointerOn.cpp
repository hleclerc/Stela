#include "InstVisitor.h"
#include "PointerOn.h"
#include "Slice.h"
#include "Inst_.h"

/**
*/
class PointerOn : public Inst_<1,1> {
public:
    virtual int size_in_bits( int nout ) const { return ptr_size; }
    virtual void write_to_stream( Stream &os ) const { os << "pointer_on(" << inp_expr( 0 ) << ")"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.pointer_on( *this, ptr_size ); }
    virtual int inst_id() const { return Inst::Id_PointerOn; }
    virtual Expr _smp_val_at( int nout, int size ) {
        return slice( inp_expr( 0 ), 0, size );
    }
    virtual const PI8 *cst_data_ValAt( int nout, int off ) const {
        return inp_expr( 0 ).cst_data() + off;
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

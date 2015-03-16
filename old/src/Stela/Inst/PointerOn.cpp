#include "InstVisitor.h"
#include "PointerOn.h"
#include "Slice.h"
#include "Inst_.h"
#include "Arch.h"

/**
*/
class PointerOn : public Inst_<1,1> {
public:
    virtual int size_in_bits( int nout ) const { return arch->ptr_size; }
    virtual void write_dot( Stream &os ) const { os << "pointer_on"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.pointer_on( *this ); }
    virtual int inst_id() const { return Inst::Id_PointerOn; }
    virtual Expr _smp_val_at( int nout, int beg, int end ) { return slice( inp_expr( 0 ), beg, end ); }
    virtual const PI8 *vat_data( int nout, int beg, int end ) const {
        return inp_expr( 0 ).cst_data( beg, end );
    }
};


Expr pointer_on( Expr expr ) {
    // simplifications
    if ( Expr res = expr.inst->_smp_pointer_on( expr.nout ) ) {
        ASSERT( arch->ptr_size == res.size_in_bits(), "..." );
        return res;
    }

    //
    PointerOn *res = new PointerOn;
    res->inp_repl( 0, expr );
    return Expr( Inst::factorized( res ), 0 );
}

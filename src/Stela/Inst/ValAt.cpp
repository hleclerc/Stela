#include "InstVisitor.h"
#include "ValAt.h"
#include "Slice.h"
#include "Inst_.h"

/**
*/
class ValAt : public Inst_<1,1> {
public:
    virtual int size_in_bits( int nout ) const { return size; }
    virtual void write_to_stream( Stream &os ) const { os << "val_at(" << inp_expr( 0 ) << "," << size << ")"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.val_at( *this, size ); }
    virtual int inst_id() const { return Inst::Id_ValAt; }

    virtual const PI8 *cst_data( int nout ) const {
        // add( ptr, int ) or add( int, ptr )
        Expr expr = inp_expr( 0 );
        return expr.inst->cst_data_ValAt( expr.nout, 0 );
    }

    virtual Expr _smp_pointer_on( int nout ) {
        return inp_expr( 0 );
    }
    int size;
};


Expr val_at( Expr expr, int size ) {
    // simplifications ?
    if ( Expr res = expr.inst->_smp_val_at( expr.nout, size ) )
        return res;

    //
    ValAt *res = new ValAt;
    res->size = size;
    res->inp_repl( 0, expr );
    return Expr( Inst::factorized( res ), 0 );
}

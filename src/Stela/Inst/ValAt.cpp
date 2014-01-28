#include "InstVisitor.h"
#include "ValAt.h"
#include "Slice.h"
#include "Inst_.h"

/**
*/
class ValAt : public Inst_<1,1> {
public:
    virtual int size_in_bits( int nout ) const { return end - beg; }
    virtual void write_to_stream( Stream &os ) const { os << "val_at(" << inp_expr( 0 ) << "," << beg << "," << end << ")"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.val_at( *this, beg, end ); }
    virtual int inst_id() const { return Inst::Id_ValAt; }

    virtual bool equal( const Inst *b ) const {
        return Inst::equal( b ) and
            beg == static_cast<const ValAt *>( b )->beg and
            end == static_cast<const ValAt *>( b )->end;
    }

    virtual const PI8 *cst_data( int nout, int lbeg, int lend ) const {
        return inp_expr( 0 ).vat_data( beg + lbeg, beg + lend );
    }

    virtual Expr _smp_pointer_on( int nout ) {
        /// pointer_on val_at ptr -> ptr
        if ( beg == 0 )
            return inp_expr( 0 ); // ->
        TODO; // -> ptr + beg
        return Expr();
    }

    int beg, end;
};


Expr val_at( Expr expr, int beg, int end ) {
    // simplifications ?
    if ( Expr res = expr.inst->_smp_val_at( expr.nout, beg, end ) )
        return res;

    //
    ValAt *res = new ValAt;
    res->beg = beg;
    res->end = end;
    res->inp_repl( 0, expr );
    return Expr( Inst::factorized( res ), 0 );
}

Expr val_at( Expr expr, int end ) {
    return val_at( expr, 0, end );
}

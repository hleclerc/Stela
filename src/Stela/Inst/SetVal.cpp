#include "InstVisitor.h"
#include "SetVal.h"
#include "Inst_.h"

///
class SetVal : public Inst_<1,3> {
public:
    virtual int size_in_bits( int nout ) const { return inp_expr( 0 ).size_in_bits(); }
    virtual void write_dot( Stream &os ) const { os << "setval"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.setval( *this ); }
    virtual int inst_id() const { return Inst::Id_SetVal; }
};

Expr setval( Expr a, Expr b, Expr beg ) {
    if ( a.size_in_bits() == 0 or b.size_in_bits() == 0 )
        return a;

    // simplifications ?
    int off;
    if ( beg.get_val( off ) ) {
        TODO;
    }

    // else create a new inst
    SetVal *res = new SetVal;
    res->inp_repl( 0, a );
    res->inp_repl( 1, b );
    res->inp_repl( 2, beg );
    return Expr( Inst::factorized( res ), 0 );
}


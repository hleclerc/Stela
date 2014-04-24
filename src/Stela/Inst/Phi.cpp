#include "InstVisitor.h"
#include "BaseType.h"
#include "Inst_.h"
#include "Phi.h"
#include "Cst.h"
#include "Op.h"

class Phi : public Inst_<1,3> {
public:
    virtual int size_in_bits( int nout ) const { return inp_expr( 1 ).size_in_bits(); }
    virtual void write_dot( Stream &os ) const { os << "phi"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.phi( *this ); }
    virtual int inst_id() const { return Inst::Id_Phi; }
    //virtual const BaseType *out_bt( int n ) const { return inp_expr( 1 ).out_bt(); }
    virtual Expr _smp_slice( int nout, int beg, int end ) {
        Expr sok = inp[ 1 ].inst->_smp_slice( inp[ 1 ].nout, beg, end );
        Expr sko = inp[ 2 ].inst->_smp_slice( inp[ 2 ].nout, beg, end );
        if ( sok or sko )
            return phi( inp[ 0 ], sok, sko );
        return Expr();
    }
};

Expr phi( Expr cond, Expr ok, Expr ko ) {
    // known value ?
    bool v;
    if ( cond.get_val( v ) )
        return v ? ok : ko;

    // the same value in all the cases ?
    if ( ok == ko )
        return ok;

    // phi( not c, a, b ) -> phi( c, b, a )
    if ( cond.inst->inst_id() == Inst::Id_Op_not )
        return phi( cond.inst->inp_expr( 0 ), ko, ok );

    // phi( c, true, false ) -> c
    // phi( c, false, true ) -> not c
    if ( ok.size_in_bits() == 1 and ko.size_in_bits() == 1 ) {
        bool va, vb;
        if ( ok.get_val( va ) and ko.get_val( vb ) ) {
            if ( va and not vb )
                return cond;
            if ( vb and not va )
                return op_not( bt_Bool, cond );
        }
    }

    // phi( cond, ok, phi( cond, a, b ) ) -> phi( cond, ok, b )
    if ( ko.inst->inst_id() == Inst::Id_Phi and ko.inst->inp_expr( 0 ) == cond )
        return phi( cond, ok, ko.inst->inp_expr( 2 ) );

    // cond = bool( ... )
    //if ( cond.inst->inst_id() == Inst::Id_Conv and cond.inst->out_bt( 0 ) == bt_Bool )
    //    cond = cond.inst->inp_expr( 0 );

    // else, create a new inst
    Phi *res = new Phi;
    res->inp_repl( 0, cond );
    res->inp_repl( 1, ok );
    res->inp_repl( 2, ko );
    return Expr( Inst::factorized( res ), 0 );
}


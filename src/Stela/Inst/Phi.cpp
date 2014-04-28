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

// return expr, knowing than cond == checked
static Expr phi_with_cond( const Expr &cond, bool checked, const Expr &expr ) {
    if ( expr.inst->inst_id() == Inst::Id_Phi ) {
        if ( expr.inst->inp_expr( 0 ) == cond )
            return checked ? expr.inst->inp_expr( 1 ) : expr.inst->inp_expr( 2 );
        Expr nok = phi_with_cond( cond, checked, expr.inst->inp_expr( 1 ) );
        Expr nko = phi_with_cond( cond, checked, expr.inst->inp_expr( 2 ) );
        if ( nok or nko )
            return phi( expr.inst->inp_expr( 0 ),
                        nok ? nok : expr.inst->inp_expr( 1 ),
                        nko ? nko : expr.inst->inp_expr( 2 )
                        );
    }
    return Expr();
}

Expr phi( Expr cond, Expr ok, Expr ko ) {
    // known value ?
    bool v;
    if ( cond.get_val( v ) )
        return v ? ok : ko;

    // the same value in all the cases ?
    if ( ok == ko )
        return ok;

    // if ok or ko are undefined
    if ( ok.inst->undefined() )
        return ko;
    if ( ko.inst->undefined() )
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

    // phi( c, b, c )
    if ( cond == ok )
        TODO;

    // phi( c, c, b )
    if ( cond == ko )
        TODO;


    // if ok is a phi node tree which contain cond in the conditions
    if ( Expr nok = phi_with_cond( cond, 1, ok ) )
        return phi( cond, nok, ko );

    // if ko is a phi node tree which contain cond in the conditions
    if ( Expr nko = phi_with_cond( cond, 0, ko ) )
        return phi( cond, ok, nko );

    // phi( cond, a, phi( cond,  ) )

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


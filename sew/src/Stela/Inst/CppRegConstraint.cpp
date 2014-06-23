#include "CppRegConstraint.h"

void CppRegConstraint::add( SI32 level, Expr expr_0, Expr expr_1, bool equ ) {
    Constraint *c_0 = constraints[ expr_0 ].push_back();
    c_0->expr  = expr_1;
    c_0->level = level;
    c_0->equ   = equ;

    Constraint *c_1 = constraints[ expr_1 ].push_back();
    c_1->expr  = expr_0;
    c_1->level = level;
    c_1->equ   = equ;
}

void CppRegConstraint::add_equ( SI32 level, Expr expr_0, Expr expr_1 ) {
    return add( level, expr_0, expr_1, true );
}

void CppRegConstraint::add_neq( SI32 level, Expr expr_0, Expr expr_1 ) {
    return add( level, expr_0, expr_1, false );
}

void CppRegConstraint::write_to_stream( Stream &os ) const {
    for( auto c : constraints ) {
        os << "\n    " << c.first << " -> ";
        for( Constraint &d : c.second )
            os << ( d.equ ? "(==,l=" : "(!=,l=" ) << d.level << ")" << d.expr;
    }
}

const Vec<CppRegConstraint::Constraint> *CppRegConstraint::get_constraints_for( Expr expr ) {
    auto res = constraints.find( expr );
    return res != constraints.end() ? &res->second : 0;
}

CppOutReg *CppRegConstraint::compulsory_reg( Expr expr ) {
    // to avoid an infinite loop
    if ( expr->op_id == Inst::cur_op_id )
        return 0;
    expr->op_id = Inst::cur_op_id;

    if ( expr->out_reg )
        return expr->out_reg;

    // output constraint ?
    const Vec<Constraint> *c = get_constraints_for( expr );
    if ( c ) {
        for( const Constraint &d : *c ) {
            if ( d.equ ) {
                if ( CppOutReg *res = compulsory_reg( d.expr ) )
                    return res;
            } else
                TODO;
        }
    }

    return 0;
}





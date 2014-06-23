#include "CppRegConstraint.h"

void CppRegConstraint::add( SI32 level, Expr expr_0, SI32 ninp_0, Expr expr_1, SI32 ninp_1, bool equ ) {
    PlugWithLevel &p_0 = constraints[ Plug{ expr_0, ninp_0 } ];
    p_0.expr  = expr_1;
    p_0.ninp  = ninp_1;
    p_0.level = level;
    p_0.equ   = equ;

    PlugWithLevel &p_1 = constraints[ Plug{ expr_1, ninp_1 } ];
    p_1.expr  = expr_0;
    p_1.ninp  = ninp_0;
    p_1.level = level;
    p_1.equ   = equ;
}

void CppRegConstraint::add_equ( SI32 level, Expr expr_0, SI32 ninp_0, Expr expr_1, SI32 ninp_1 ) {
    return add( level, expr_0, ninp_0, expr_1, ninp_1, true );
}

void CppRegConstraint::add_neq( SI32 level, Expr expr_0, SI32 ninp_0, Expr expr_1, SI32 ninp_1 ) {
    return add( level, expr_0, ninp_0, expr_1, ninp_1, false );
}

void CppRegConstraint::write_to_stream( Stream &os ) const {
    for( auto c : constraints ) {
        os << "\n    ";

        os << c.first.expr;
        if ( c.first.ninp < 0 )
            os << " out";
        else
            os << "[" << c.first.ninp << "]";

        os << " -> ";

        os << c.second.expr;
        if ( c.second.ninp < 0 )
            os << " out";
        else
            os << "[" << c.second.ninp << "]";

        os << " (level=" << c.second.level << ")";
    }
}

const CppRegConstraint::PlugWithLevel *CppRegConstraint::get_out_constraint( Expr expr ) {
    auto res = constraints.find( Plug{ expr, -1 } );
    return res != constraints.end() ? &res->second : 0;
}

const CppRegConstraint::PlugWithLevel *CppRegConstraint::get_inp_constraint( Expr expr, int ninp ) {
    auto res = constraints.find( Plug{ expr, ninp } );
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
    const PlugWithLevel *c = get_out_constraint( expr );
    if ( c ) {
        if ( c->equ ) {
            if ( CppOutReg *res = compulsory_reg( c->ninp >= 0 ? c->expr->inp[ c->ninp ] : c->expr ) )
                return res;
        } else
            TODO;
    }

    // input constraint ?
    for( Inst::Parent &p : expr->par ) {
        const PlugWithLevel *c = get_inp_constraint( p.inst, p.ninp );
        if ( c ) {
            if ( c->equ ) {
                if ( CppOutReg *res = compulsory_reg( c->ninp >= 0 ? c->expr->inp[ c->ninp ] : c->expr ) )
                    return res;
            } else
                TODO;
        }
    }
    return 0;
}





#include "../System/BinStreamReader.h"
#include "../Ir/CallableFlags.h"
#include "../Inst/Cst.h"

#include "CallableInfo_Class.h"
#include "Interpreter.h"
#include "Scope.h"

#include <limits>

CallableInfo_Class::CallableInfo_Class( Interpreter *ip, const PI8 *sf, const PI8 *tok_data, int src_off ) : CallableInfo_WT( sf, src_off ) {
    BinStreamReader bin( tok_data + 1 );
    parse_wt( ip, sf, bin );

    for( int i = 0, nb_anc = bin.read_positive_integer(); i < nb_anc; ++i )
        ancestors << Code( sf, bin.read_offset() );
}

CallableInfo::Trial *CallableInfo_Class::test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off, Scope *caller ) {
    Interpreter *ip = caller->interpreter();
    TrialClass *res = new TrialClass;

    if ( flags & IR_HAS_COMPUTED_PERT ) return res->wr( "TODO: computed pertinence" );
    // nb arguments
    if ( pnu + pnn < min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn > max_nb_args() ) return res->wr( "To much arguments" );

    Scope scope = Scope( ip, ip->main_scope, caller );

    res->args.resize( arg_names.size() );
    if ( has_varargs() ) {
        TODO;
    }
    TODO;

    return res;
}

void CallableInfo_Class::TrialClass::call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off, Var &res, Expr cond, Scope *caller ) {
    Interpreter *ip = caller->interpreter();
    // TypeInfo
    //    Vec<Var>  parameters;
    //    Var       type_var;


    res = Var( ip, &ip->type_SI32, cst( 17 ) );
}

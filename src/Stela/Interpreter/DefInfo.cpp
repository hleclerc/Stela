#include "../System/BinStreamReader.h"
#include "../Ir/CallableFlags.h"
#include "../Inst/Cst.h"

#include "Interpreter.h"
#include "DefInfo.h"
#include "Scope.h"

DefInfo::DefInfo( const Expr *sf, int src_off, BinStreamReader bin ) : CallableInfo_WT( sf, src_off, bin ) {
    block_with_ret = Code( sf, bin.read_offset() );
    if ( flags & IR_HAS_RETURN_TYPE )
        return_type = Code( sf, bin.read_offset() );

    get_of = flags & IR_IS_A_GET ? ip->glo_nstr( sf, bin.read_positive_integer() ) : -1;
    set_of = flags & IR_IS_A_SET ? ip->glo_nstr( sf, bin.read_positive_integer() ) : -1;
    sop_of = flags & IR_IS_A_SOP ? ip->glo_nstr( sf, bin.read_positive_integer() ) : -1;

    //    if ( d->get_of >= 0 or d->set_of >= 0 or d->sop_of >= 0 ) {
    //        SI32 get_of = d->get_of >= 0 ? d->name : -1;
    //        SI32 set_of = d->set_of >= 0 ? d->name : -1;
    //        SI32 sop_of = d->sop_of >= 0 ? d->name : -1;

    //        SI32 orig = std::max( d->get_of, std::max( d->set_of, d->sop_of ) );
    //        if ( Var res = named_vars.get( orig ) ) {
    //            if ( res.type->base_class != &ip->class_GetSetSopDef )
    //                write_error( "There's already a variable named '" + glob_nstr_cor.str( orig ) + "' in current scope (not of type GetSetSopDef)", sf, d->off );
    //            else {
    //                if ( get_of < 0 ) get_of = to_int( res.type->parameters[ 0 ] );
    //                if ( set_of < 0 ) set_of = to_int( res.type->parameters[ 1 ] );
    //                if ( sop_of < 0 ) sop_of = to_int( res.type->parameters[ 2 ] );
    //            }
    //        }

    //        Vec<Var> par;
    //        par << make_int_var( get_of );
    //        par << make_int_var( set_of );
    //        par << make_int_var( sop_of );

    //        Var res;
    //        res.type = ip->class_GetSetSopDef.type_for( this, par, sf, d->off );
    //        res.flags = Var::PARTIAL_INSTANCIATION;

    //        reg_var( orig, res, true, false, sf, d->off );
    //    }
}

CallableInfo::Trial *DefInfo::test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Expr *sf, int off, Scope *caller ) {
    TrialDef *res = new TrialDef;
    return res->wr( "TODO" );
}

void DefInfo::TrialDef::call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Expr *sf, int off, Var &res, Expr cond, Scope *caller ) {
    TODO;
}

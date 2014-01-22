#include "../System/BinStreamReader.h"
#include "../Ir/CallableFlags.h"
#include "CallableInfo.h"
#include "Interpreter.h"
#include "SourceFile.h"

CallableInfo::Trial::Trial() : reason( 0 ) {
}

// CallableInfo_WT
CallableInfo_WT::CallableInfo_WT( const PI8 *sf, int src_off ) : sf( sf ), src_off( src_off ) {
}

void CallableInfo_WT::parse_wt( Interpreter *ip, const PI8 *sf, BinStreamReader &bin ) {
    src_off = bin.read_positive_integer();
    name  = ip->glo_nstr( sf, bin.read_positive_integer() );
    flags = bin.read_positive_integer();
    nargs = bin.read_positive_integer();
    dargs = bin.read_positive_integer();
    pertinence = flags & IR_HAS_COMPUTED_PERT ? 0 : FP64( bin.read_positive_integer() ) / FP64( bin.read_positive_integer() );
    if ( flags & IR_NEG_PERT )
        pertinence = - pertinence;
    for( int i = 0; i < nargs; ++i )
        arg_names << ip->glo_nstr( sf, bin.read_positive_integer() );
    for( int i = 0; i < dargs; ++i )
        arg_defaults << Code( sf, bin.read_offset() );
    if ( flags & IR_HAS_COMPUTED_PERT )
        comp_pert = Code( sf, bin.read_offset() );
    if ( flags & IR_HAS_CONDITION )
        condition = Code( sf, bin.read_offset() );
    block = Code( sf, bin.read_offset() );

    // d->sf  = sf;
    // d->off = bin.read_positive_integer();

}

const char *CallableInfo_WT::filename() const {
    return SourceFile( sf ).filename();
}

int CallableInfo_WT::off() const {
    return src_off;
}

// CallableInfo_Class
CallableInfo_Class::CallableInfo_Class( Interpreter *ip, const PI8 *sf, const PI8 *tok_data, int src_off ) : CallableInfo_WT( sf, src_off ) {
    BinStreamReader bin( tok_data + 1 );
    parse_wt( ip, sf, bin );

    for( int i = 0, nb_anc = bin.read_positive_integer(); i < nb_anc; ++i )
        ancestors << Code( sf, bin.read_offset() );
}

void CallableInfo_Class::test( Trial &trial, int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off ) {
    trial.reason = "TODODO";
}


// CallableInfo_Def
CallableInfo_Def::CallableInfo_Def( Interpreter *ip, const PI8 *sf, const PI8 *tok_data, int src_off ) : CallableInfo_WT( sf, src_off ) {
    BinStreamReader bin( tok_data );
    parse_wt( ip, sf, bin );

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

void CallableInfo_Def::test( Trial &trial, int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off ) {
    trial.reason = "TODODO";
}

#include "../System/BinStreamReader.h"
#include "../Ir/CallableFlags.h"
#include "CallableInfo_WT.h"
#include "SourceFile.h"
#include "Interpreter.h"
#include <limits>

CallableInfo_WT::CallableInfo_WT( const Expr *sf, const PI8 *tok_data, int src_off ) : sf( sf ), tok_data( tok_data ), src_off( src_off ) {
}

void CallableInfo_WT::parse_wt( Interpreter *ip, const Expr *sf, BinStreamReader &bin ) {
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
    return SourceFile( sf->cst_data() ).filename();
}

int CallableInfo_WT::off() const {
    return src_off;
}

int CallableInfo_WT::min_nb_args() const {
    return arg_names.size() - arg_defaults.size();
}

int CallableInfo_WT::max_nb_args() const {
    if ( has_varargs() )
        return std::numeric_limits<int>::max();
    return arg_names.size();
}

bool CallableInfo_WT::has_varargs() const {
    return flags & IR_VARARGS;
}

bool CallableInfo_WT::self_as_arg() const {
    return flags & IR_SELF_AS_ARG;
}

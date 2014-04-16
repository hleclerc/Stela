#include "../System/BinStreamReader.h"
#include "../Ir/CallableFlags.h"
#include "CallableInfo_WT.h"
#include "Interpreter.h"
#include <limits>

CallableInfo_WT::CallableInfo_WT( const Expr &sf, int src_off, BinStreamReader &bin, VarTable *sn ) : sf( sf ), src_off( src_off ), sn( sn ) {
    name  = ip->glo_nstr( sf, bin.read_positive_integer() );
    flags = bin.read_positive_integer();
    nargs = bin.read_positive_integer();
    dargs = bin.read_positive_integer();
    pertinence = flags & IR_HAS_COMPUTED_PERT ? 0 : FP64( bin.read_positive_integer() ) / FP64( bin.read_positive_integer() );
    if ( flags & IR_NEG_PERT )
        pertinence = - pertinence;
    for( int i = 0; i < nargs; ++i ) {
        // arg name
        arg_names << ip->glo_nstr( sf, bin.read_positive_integer() );

        // constraints
        ArgCst *ac = arg_constraints.push_back();
        int nb = bin.read_positive_integer();
        for( int i = 0; i < nb; ++i )
            ac->class_names << ip->glo_nstr( sf, bin.read_positive_integer() );
    }
    for( int i = 0; i < dargs; ++i )
        arg_defaults << Code( sf, bin.read_offset() );
    if ( flags & IR_HAS_COMPUTED_PERT )
        comp_pert = Code( sf, bin.read_offset() );
    if ( flags & IR_HAS_CONDITION )
        condition = Code( sf, bin.read_offset() );
    block = Code( sf, bin.read_offset() );
}

const char *CallableInfo_WT::filename() const {
    return ip->sf_info( sf )->filename;
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

int CallableInfo_WT::nb_arg_vals() const {
    return self_as_arg() + arg_names.size() + has_varargs();
}


bool CallableInfo_WT::has_varargs() const {
    return flags & IR_VARARGS;
}

bool CallableInfo_WT::self_as_arg() const {
    return flags & IR_SELF_AS_ARG;
}

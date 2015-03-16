#include "../Ir/CallableFlags.h"
#include "Callable.h"
#include <limits>
#include "Ip.h"

Callable::Trial::Trial( const char *reason ) : reason( reason ) {
}

Callable::Trial::~Trial() {
}

Var Callable::Trial::call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self, int apply_mode ) {
    ERROR( "weird... should not be here" );
    return ip->error_var();
}



Callable::Callable() {
}

void Callable::read_bin( BinStreamReader &bin ) {
    flags = bin.read_positive_integer();
    nargs = bin.read_positive_integer();
    dargs = bin.read_positive_integer();
    pertinence = flags & IR_HAS_COMPUTED_PERT ? 0 : FP64( bin.read_positive_integer() ) / FP64( bin.read_positive_integer() );
    if ( flags & IR_NEG_PERT )
        pertinence = - pertinence;
    for( int i = 0; i < nargs; ++i ) {
        // arg name
        arg_names << ip->read_nstring( bin );

        // constraints
        ArgCst *ac = arg_constraints.push_back();
        int nb = bin.read_positive_integer();
        for( int i = 0; i < nb; ++i )
            ac->class_names << ip->read_nstring( bin );
    }
    for( int i = 0; i < dargs; ++i )
        arg_defaults << Code( sf, bin.read_offset() );
    if ( flags & IR_HAS_COMPUTED_PERT )
        comp_pert = Code( sf, bin.read_offset() );
    if ( flags & IR_HAS_CONDITION )
        condition = Code( sf, bin.read_offset() );
    block = Code( sf, bin.read_offset() );
}

int Callable::min_nb_args() const {
    return arg_names.size() - arg_defaults.size();
}

int Callable::max_nb_args() const {
    if ( has_varargs() )
        return std::numeric_limits<int>::max();
    return arg_names.size();
}

int Callable::nb_arg_vals() const {
    return self_as_arg() + arg_names.size() + has_varargs();
}


bool Callable::has_varargs() const {
    return flags & IR_VARARGS;
}

bool Callable::self_as_arg() const {
    return flags & IR_SELF_AS_ARG;
}

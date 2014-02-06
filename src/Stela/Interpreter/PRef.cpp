#include "Var.h"

PRef::~PRef() {
    delete refs;
}

void PRef::add_ref( Var var ) {
    if ( not refs )
        refs = new Vec<Var>;
    *refs << var;
}

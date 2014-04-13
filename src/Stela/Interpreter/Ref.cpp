#include "Ref.h"

Ref::Ref() {
}

Ref::~Ref() {
}

bool Ref::indirect_set( Expr expr, Scope *set_scope, const Expr &sf, int off, Expr ext_cond ) {
    return false;
}

void Ref::direct_set( Expr expr ) {
    ERROR( "should not be here" );
}


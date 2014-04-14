#include "Ref.h"
#include "Var.h"

Ref::Ref() {
}

Ref::~Ref() {
}

bool Ref::indirect_set( const Var &src, Scope *set_scope, const Expr &sf, int off, Expr ext_cond ) {
    return false;
}

Var Ref::pointed_value() const {
    TODO;
    return Var();
}

#include "BaseType.h"
#include "Arch.h"
#include "Cst.h"

Arch::Arch() {
    ptr_size = 8 * sizeof( void *);
    ptr_alig = 32;
    rev_endn = false;
}

Expr Arch::cst_ptr( SI64 val ) {
    if ( ptr_size == 32 )
        return cst( SI32( val ) );
    return cst( val );
}

const BaseType *Arch::bt_ptr() {
    if ( ptr_size == 32 )
        return bt_SI32;
    return bt_SI64;
}

static Arch arch_inst;
Arch *arch = &arch_inst;

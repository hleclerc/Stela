#include "../Inst/PointerOn.h"
#include "../Inst/Arch.h"
#include "RefPointerOn.h"
#include "Interpreter.h"

RefPointerOn::RefPointerOn( Var var, int ptr_size ) : var( var ), ptr_size( ptr_size ) {
}

Expr RefPointerOn::expr() const {
    return pointer_on( var.expr() );
}

void RefPointerOn::set( Expr expr ) {
    TODO;
}

void RefPointerOn::write_to_stream( Stream &os ) const {
    os << "&" << var;
}

Var pointer_on( Var var ) {
    Var type_var = ip->type_of( var );
    return Var( ip->type_for( ip->class_info( ip->class_Ptr.expr() ), &type_var ), new RefPointerOn( var, arch->ptr_size ) );
}

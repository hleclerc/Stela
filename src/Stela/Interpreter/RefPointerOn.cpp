#include "../Inst/PointerOn.h"
#include "../System/dcast.h"
#include "../Inst/Arch.h"
#include "RefPointerOn.h"
#include "Interpreter.h"
#include "RefPhi.h"
#include "Scope.h"

RefPointerOn::RefPointerOn( Var var ) : var( var ) {
}

Expr RefPointerOn::expr() const {
    return pointer_on( var.expr() );
}

//bool RefPointerOn::indirect_set( const Var &src, Scope *set_scope, const Expr &sf, int off, Expr ext_cond ) {
//    // var doit désigner une nouvelle référence -> il faut un nouveau PRef
//    // -> comment revenir à l'ancien ?
//    //   -> prop 1 :
//    set_scope->set( var, src.pointed_value(), sf, off, ext_cond, false );
//    return true;
//}

void RefPointerOn::write_to_stream( Stream &os ) const {
    os << "&" << var;
}

Var RefPointerOn::pointed_value() const {
    return var;
}

Var pointer_on( Var var ) {
    Var type_var = ip->type_of( var );
    return Var( ip->type_for( ip->class_info( ip->class_Ptr.expr() ), &type_var ), new RefPointerOn( var ) );
}

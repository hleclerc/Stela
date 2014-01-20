#include "Interpreter.h"
#include "InstPtr.h"
#include "RefPtr.h"

RefPtr::RefPtr( Var var, int ptr_size ) : var( var ), ptr_size( ptr_size ) {
}

Expr RefPtr::get() const {
    return ptr( var.get(), ptr_size );
}

void RefPtr::set( Expr expr ) {
    TODO;
}

void RefPtr::write_to_stream( Stream &os ) const {
    os << "&" << var;
}

Var ptr( Interpreter *ip, Var var ) {
    Var type_var = ip->type_of( var );
    return Var( ip, ip->type_for( ip->class_info( ip->class_Ptr ), &type_var ), new RefPtr( var, ip->ptr_size() ) );
}

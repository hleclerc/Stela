#include "Room.h"
#include "Type.h"
#include "Var.h"
#include "Op.h"
#include "Ip.h"

Var::Var( Ref, Type *type, Expr ptr ) : type( type ), inst( ptr ) {
}

Var::Var( Type *type, Expr val ) : type( type ), inst( room( type->size(), val ) ) {
}

Var::Var( Type *type ) : type( type ), inst( room( type->size() ) ) {
}

void Var::write_to_stream( Stream &os ) const {
    os << "{" << *type << "}(" << inst << ')';
}

Expr Var::operator*() const {
    return get_val();
}

Var::PVar Var::operator*() {
    return PVar{ this };
}

Expr Var::get_val() const {
    return simplified( inst->_get_val() );
}

void Var::set_val( Var val ) {
    set_val( *val );
}

void Var::set_val( Expr val ) {
    inst->_set_val( val );
}

Var ptr( const Var &val ) {
    return Var( &ip->type_RawPtr, val.inst );
}

Var at( Type *type, Var ptr ) {
    Expr res = ptr.inst->_at( type->size() );
    if ( not res->is_a_pointer() )
        return ip->ret_error( "at requires a pointer var" );
    return Var( Ref(), type, res );
}

Var and_boolean( Var a, Var b ) {
    return Var( &ip->type_Bool, op( &ip->type_Bool, a.type, a.get_val(), b.type, b.get_val(), Op_and_boolean() ) );
}


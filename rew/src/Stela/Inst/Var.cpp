#include "SelectDep.h"
#include "Syscall.h"
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

Expr Var::get_val() {
    return simplified( inst->_get_val() );
}

void Var::set_val( Var val ) {
    if ( type != val.type )
        TODO;
    if ( not type->pod() )
        TODO;
    set_val( val.get_val() );
}

void Var::set_val( Expr val ) {
    inst->_set_val( val );
}

Var Var::ptr() {
    return Var( &ip->type_RawPtr, inst );
}

Var Var::at( Type *type ) {
    Expr res = inst->_at( type->size() );
    if ( not res->is_a_pointer() )
        return ip->ret_error( "at requires a pointer var" );
    return Var( Ref(), type, res );
}

Var Var::and_boolean( Var b ) {
    return Var( &ip->type_Bool, op( &ip->type_Bool, type, get_val(), b.type, b.get_val(), Op_and_boolean() ) );
}

Var Var::or_boolean( Var b ) {
    return Var( &ip->type_Bool, op( &ip->type_Bool, type, get_val(), b.type, b.get_val(), Op_or_boolean() ) );
}

Var Var::not_boolean() {
    return Var( &ip->type_Bool, op( &ip->type_Bool, type, get_val(), Op_not_boolean() ) );
}

struct AddStoreDep : public Inst::Visitor {
    virtual void operator()( Expr expr ) {
        expr->_add_store_dep_if_necessary( res, fut );
    }
    Expr res, fut;
};

Var syscall( const Vec<Var> &inp ) {
    Vec<Expr> inp_expr;
    for( Var i : inp )
        inp_expr << i.get_val();
    Expr res = syscall( inp_expr, ip->sys_state.inst );
    Expr fut = select_dep( ip->cur_cond(), res, ip->sys_state.inst );

    // each time we see a pointer on something, we have to add a store a dep
    AddStoreDep add_store_dep;
    add_store_dep.res = res;
    add_store_dep.fut = fut;
    ++Inst::cur_op_id;
    for( Expr expr : inp_expr )
        expr->visit( add_store_dep );

    ip->sys_state.inst = fut;
    return Var( ip->type_ST, res );
}

#include "SelectDep.h"
#include "Syscall.h"
#include "Room.h"
#include "Conv.h"
#include "Type.h"
#include "Var.h"
#include "Cst.h"
#include "Op.h"
#include "Ip.h"

Var::Var( Ref, Type *type, Expr ptr ) : type( type ), inst( ptr ), flags( 0 ) {
}

Var::Var( Type *type, Expr val ) : type( type ), inst( room( type->size(), val ) ), flags( 0 ) {
}

Var::Var( Type *type ) : type( type ), inst( room( type->size() ) ), flags( 0 ) {
}

Var::Var() : type( 0 ), flags( 0 ) {
}

Var::Var( SI64 val ) : Var( &ip->type_SI64, cst( 64, (const PI8 *)&val ) ) {
}

void Var::write_to_stream( Stream &os ) const {
    Expr v = const_cast<Var *>( this )->get_val();
    if ( type == &ip->type_Type ) {
        SI64 p;
        if ( v->get_val( p ) )
            os << *reinterpret_cast<Type *>( ST( p ) );
        return;
    }
    // os << *type << "{" << v << '}';
    os << *type << "{" << inst << '}';
}

Expr Var::get_val() {
    return simplified( inst->_get_val( type->size() ) );
}

bool Var::get_val( SI32 &val ) {
    if ( Expr e = get_val() )
        return e->get_val( val, type );
    return false;
}

bool Var::always( Bool val ) {
    if ( type != &ip->type_Bool )
        return conv( &ip->type_Bool ).always( val );
    Bool res;
    return get_val()->get_val( res ) and res == val;
}

Expr Var::ref() {
    return inst;
}

bool Var::is_surdef() const {
    return flags & SURDEF;
}

bool Var::is_weak_const() const {
    return flags & WEAK_CONST;
}

bool Var::is_an_error() const {
    return type == &ip->type_Error;
}

bool Var::defined() const {
    return type and inst;
}


void Var::set_val( Var val, Rese, Expr cond ) {
    if ( type == &ip->type_Error or val.type == &ip->type_Error )
        return;
    if ( not type ) {
        type = val.type;
        inst = val.inst;
        return;
    }
    if ( type != val.type or not type->pod() ) {
        if ( type->_aryth and val.type->_aryth ) {
            set_val( val.conv( type ), Rese(), cond );
            return;
        }
        ip->main_scope->apply( ip->main_scope->get_attr( *this, STRING_reassign_NUM ), 1, &val );
        return;
    }
    set_val( val.get_val(), Rese(), cond );
}

void Var::set_val( int offset, Var val, Rese, Expr cond ) {
    ( ptr() + Var( offset ) ).at( val.type ).set_val( val, Rese(), cond );
}

void Var::set_val( int offset, Type *type, Expr val, Rese, Expr cond ) {
    ( ptr() + Var( offset ) ).at( type ).set_val( val, Rese(), cond );
}

void Var::set_val( Expr val, Rese, Expr cond ) {
    if ( flags & WEAK_CONST )
        return ip->disp_error( "attempting to modify a const value" );
    if ( inst )
        inst->_set_val( val, type->size(), Rese(), cond );
    else
        inst = val;
}

Var Var::ptr() {
    return Var( ip->type_ST, inst );
}

Var Var::at( Type *target_type ) {
    Expr res = simplified( inst->_get_val() );
    if ( not res->is_a_pointer() ) {
        std::cerr << res << std::endl;
        return ip->ret_error( "at requires a pointer var" );
    }
    return Var( Ref(), target_type, res );
}

Var Var::conv( Type *ntype ) {
    if ( type == ntype )
        return *this;
    return Var( ntype, ::conv( ntype, type, get_val() ) );
}

Var Var::operator&&( Var b ) {
    return Var( &ip->type_Bool, op( &ip->type_Bool, type, get_val(), b.type, b.get_val(), Op_and_boolean() ) );
}

Var Var::operator||( Var b ) {
    return Var( &ip->type_Bool, op( &ip->type_Bool, type, get_val(), b.type, b.get_val(), Op_or_boolean() ) );
}

bool Var::operator==( Var b ) {
    return type == b.type and get_val() == b.get_val();
}

bool Var::operator!=( Var b ) {
    return type != b.type or get_val() != b.get_val();
}

Var Var::operator!() {
    return Var( &ip->type_Bool, op( &ip->type_Bool, type, get_val(), Op_not_boolean() ) );
}

Var Var::operator+( Var b ) {
    if ( type != b.type ) {
        PRINT( *type );
        PRINT( *b.type );
        TODO;
    }
    return Var( type, op( type, type, get_val(), b.type, b.get_val(), Op_add() ) );
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

    // each time we see a pointer on something, we have to add a store dep
    AddStoreDep add_store_dep;
    add_store_dep.res = res;
    add_store_dep.fut = fut;
    ++Inst::cur_op_id;
    for( Expr expr : inp_expr )
        expr->visit( add_store_dep, true, false );

    ip->sys_state.inst = fut;
    return Var( ip->type_ST, res );
}



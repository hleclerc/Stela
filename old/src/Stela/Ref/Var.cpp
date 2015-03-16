#include "../Inst/Conv.h"
#include "../Inst/Cst.h"
#include "../Inst/Op.h"
#include "RefItem_PointerOn.h"
#include "RefItem_Slice.h"
#include "RefItem_Expr.h"
#include "Var.h"
#include "Ip.h"

Var::Var( Type *type, PI8 *data, PI8 *knwn ) : flags( 0 ) {
    ref = new Ref( type, new RefItem_Expr( cst( data, knwn, type->size() ) ) );
}

Var::Var( SI32 val ) :
    ref( new Ref( &ip->type_SI32, new RefItem_Expr( cst( (PI8 *)&val, 0, 32 ) ) ) ),
    flags( 0 ) {
}

Var::Var( Ref *ref, int flags ) : ref( ref ), flags( flags ) {
}

Var::Var() : flags( 0 ) {
}

Var pointed_val( const Var &var ) {
    return var.ref->ref_item->pointed_val();
}

Var pointer_on( const Var &var ) {
    return Var( new Ref( ip->pointer_type( var.ref->type ), new RefItem_PointerOn( var ) ), 0 );
}

template<class Op>
Var op( const Var &a, const Var &b, Op op_n, bool boolean ) {
    if ( const BaseType *ta = a.type()->bt ) {
        if ( const BaseType *tb = b.type()->bt ) {
            const BaseType *tr = type_promote( ta, tb );
            Type *vt = boolean ? &ip->type_Bool : ip->type_for( tr );
            return Var( new Ref( vt, new RefItem_Expr( op( tr, conv( tr, ta, a.expr() ), conv( tr, tb, b.expr() ), op_n ) ) ), 0 );
        }
    }
    //
    TODO;
    return ip->error_var();
}

Var operator+( const Var &a, const Var &b ) {
    if ( a.ref->ref_item->contains_a_ptr() or b.ref->ref_item->contains_a_ptr() )
        TODO;
    return op( a, b, Op_add(), false );
}

Var operator-( const Var &a, const Var &b ) {
    if ( a.ref->ref_item->contains_a_ptr() or b.ref->ref_item->contains_a_ptr() )
        TODO;
    return op( a, b, Op_sub(), false );
}

Var slice( const Var &a, Type *type, int offset ) {
    return Var( new Ref( type, new RefItem_Slice( a, offset, offset + type->size() ) ), 0 );
}


void Var::operator+=( const Var &b ) {
    *this = *this + b;
}

void Var::operator-=( const Var &b ) {
    *this = *this - b;
}

void Var::write_to_stream( Stream &os ) const {
    os << ref;
}

void Var::operator=( const Var &src ) {
    if ( weak_const() )
        return ip->disp_error( "A const variable cannot be reassigned" );

    // void type
    if ( not ref ) {
        ref = new Ref( src.ref->type, src.ref->ref_item->shallow_copy() );
        return;
    }

    // "fundamental" reassign ?
    if ( ref->type == src.ref->type and ref->type->pod() )
        return ref->reassign( *src.ref );

    // else, call the reassign method
    TODO;

}

bool Var::contains_a_ptr() const {
    return ref->ref_item->contains_a_ptr();
}

bool Var::weak_const() const {
    return ( flags & WEAK_CONST ) or full_const();
}

bool Var::full_const() const {
    return ref and ( ref->flags & Ref::FULL_CONST );
}

bool Var::defined() const {
    return ref;
}

Var Var::copy() const {
    // "fundamental" copy ?
    return Var( new Ref( ref->type, ref->ref_item->shallow_copy() ) );
}

Expr Var::expr() const {
    return ref->expr();
}

Type *Var::type() const {
    return ref->type;
}

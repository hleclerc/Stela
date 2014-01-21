#include "../Inst/Cst.h"
#include "RefExpr.h"
#include "Var.h"

Var::Var( Interpreter *ip, Var *type, const Expr &expr ) : data( new PRef( ip ) ), type( type->data ), flags( 0 ) {
    data->ptr = new RefExpr( expr );
}

Var::Var( Interpreter *ip, Var *type, Ref *ref ) : data( new PRef( ip ) ), type( type->data ), flags( 0 ) {
    data->ptr = ref;
}

Var::Var( Interpreter *ip, Var *type ) : data( new PRef( ip ) ), type( type->data ), flags( 0 ) {
}

Var::Var() : flags( 0 ) {
}

Var::~Var() {
}

const PI8 *Var::cst_data() const {
    return data and data->ptr ? data->ptr->get().cst_data() : 0;
}

bool Var::referenced_more_than_one_time() const {
    return data->cpt_use > 1;
}

void Var::write_to_stream( Stream &os ) const {
    os << type << "(" << data << ")";
}

Expr Var::get() const {
    return data and data->ptr ? data->ptr->get() : cst( Vec<PI8>() );
}

bool Var::set( Expr expr ) {
    ASSERT( data, "..." );
    // checkings
    if ( flags & WEAK_CONST )
        return false;
    if ( data->flags & PRef::CONST )
        return false;
    // set
    if ( data->ptr )
        data->ptr->set( expr );
    else
        data->ptr = new RefExpr( expr );
    return true;
}

Var constified( const Var &var ) {
    if ( var.referenced_more_than_one_time() and not var.data->is_const() )
        ERROR( "only var that are not referenced more than one time can be fully constified" );
    Var res = var;
    res.data->flags |= PRef::CONST;
    res.flags |= Var::WEAK_CONST;
    return res;
}


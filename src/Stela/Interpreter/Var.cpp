#include "../Inst/Slice.h"
#include "../Inst/ValAt.h"
#include "../Inst/Arch.h"
#include "../Inst/Cst.h"
#include "Interpreter.h"
#include "RefExpr.h"
#include "Var.h"

Var::Var( Var *type, const Expr &expr ) : data( new PRef ), type( type->data ), flags( 0 ) {
    data->ptr = new RefExpr( expr );
}

Var::Var( Var *type, Ref *ref ) : data( new PRef ), type( type->data ), flags( 0 ) {
    data->ptr = ref;
}

Var::Var( Var *type ) : data( new PRef ), type( type->data ), flags( 0 ) {
}

Var::Var() : flags( 0 ) {
}

Var::~Var() {
}

const PI8 *Var::cst_data() const {
    return data and data->ptr ? data->ptr->expr().cst_data() : 0;
}

bool Var::referenced_more_than_one_time() const {
    return data->cpt_use > 1;
}

bool Var::is_weak_const() const {
    return flags & WEAK_CONST;
}

bool Var::is_surdef() const {
    return flags & SURDEF;
}

static void write_var( Stream &os, Expr type, Expr data ) {
    // type
    if ( type ) {
        int ps = arch->ptr_size;
        ClassInfo *ci = ip->class_info( slice( type, 0, ps ) );
        os << ip->glob_nstr_cor.str( ci->name );
        if ( ci->arg_names.size() ) {
            os << "[";
            for( int i = 0; i < ci->arg_names.size(); ++i ) {
                if ( i )
                    os << ",";
                write_var( os,
                           slice( type, ( 2 * i + 1 ) * ps, ( 2 * i + 2 ) * ps ), // type
                           slice( type, ( 2 * i + 2 ) * ps, ( 2 * i + 3 ) * ps ) // data
                           );
            }
            os << "]";
        }
    } else
        os << "UndefinedType";
    //
    os << "(" << data << ")";
}

void Var::write_to_stream( Stream &os ) const {
    write_var( os, type ? type->expr() : Expr(), data ? data->expr() : Expr() );
}

Expr Var::expr() const {
    return data and data->ptr ? data->ptr->expr() : cst( Vec<PI8>() );
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

Expr Var::type_expr() const {
    return type->ptr->expr();
}

Var constified( const Var &var ) {
    if ( var.referenced_more_than_one_time() and not var.data->is_const() ) {
        ERROR( "only var that are not referenced more than one time can be fully constified" );
        TODO;
    }
    //
    Var res = var;
    res.data->flags |= PRef::CONST;
    res.flags |= Var::WEAK_CONST;
    return res;
}


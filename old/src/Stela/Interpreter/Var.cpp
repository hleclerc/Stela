#include "../Inst/Slice.h"
#include "../Inst/ValAt.h"
#include "../Inst/Arch.h"
#include "../Inst/Cst.h"
#include "Interpreter.h"
#include "RefExpr.h"
#include "Var.h"

#include <sstream>

static void write_var_type( Stream &os, Expr type );

Var::Var( Ptr<PRef> type, Ptr<PRef> data, int flags ) : data( data ), type( type ), flags( flags ) {
}

Var::Var( Ptr<PRef> type, const Expr &expr ) : data( new PRef ), type( type ), flags( 0 ) {
    data->ptr = new RefExpr( expr );
}

Var::Var( Ptr<PRef> type, Ptr<Ref> ref ) : data( new PRef ), type( type ), flags( 0 ) {
    data->ptr = ref;
}

Var::Var( Var *type, const Expr &expr ) : data( new PRef ), type( type->data ), flags( 0 ) {
    data->ptr = new RefExpr( expr );
}

Var::Var( Var *type, Ptr<PRef> ref ) : data( ref ), type( type->data ), flags( 0 ) {
}

Var::Var( Var *type, Ptr<Ref> ref ) : data( new PRef ), type( type->data ), flags( 0 ) {
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
    return data->cpt_use > 1 or ( data->ptr and data->ptr->contains_var_referenced_more_than_one_time() );
}

Var Var::add_ref( int offset, const Var &var ) {
    if ( data and var )
        data->add_ref( offset, var );
    return *this;
}

Var Var::add_ref_from( const Var &var ) {
    if ( data and var and var.data )
        data->add_ref_from( var.data.ptr() );
    return *this;
}

Var Var::get_ref( int offset ) {
    if ( data )
        return data->get_ref( offset );
    return Var();
}

Var Var::pointed_value() const {
    return data->ptr->pointed_value();
}

ClassInfo *Var::class_info() const {
    return ip->class_info( slice( type_expr(), 0, arch->ptr_size ) );
}

TypeInfo *Var::type_info() const {
    return ip->type_info( type_expr() );
}

bool Var::is_weak_const() const {
    return flags & WEAK_CONST;
}

bool Var::is_full_const() const {
    return data and ( data->flags & PRef::CONST );
}

bool Var::is_surdef() const {
    return flags & SURDEF;
}

static void write_var( Stream &os, Expr type, Expr data, bool star = false ) {
    write_var_type( os, type );
    os << "{";
    if ( star )
        os << "*";
    os << data << "}";
}

static void write_var_type( Stream &os, Expr type ) {
    if ( type ) {
        int ps = arch->ptr_size;
        ClassInfo *ci = ip->class_info( slice( type, 0, ps ) );
        os << ip->glob_nstr_cor.str( ci->name );
        if ( ci->arg_names.size() ) {
            os << "[";
            //            for( int i = 0; i < type.size_in_bits() / ps / 2; ++i )
            //                //os << slice( type, ( 2 * i + 1 ) * ps, ( 2 * i + 2 ) * ps ) << ";";
            ////            PRINT( type.size_in_bits() / ps / 2 );
            //            for( int i = 0; i < type.size_in_bits() / ps / 2; ++i ) {
            //                if ( i )
            //                    os << ",";
            //                write_var( os,
            //                           slice( type, ( 2 * i + 1 ) * ps, ( 2 * i + 2 ) * ps ), // type
            //                           slice( type, ( 2 * i + 2 ) * ps, ( 2 * i + 3 ) * ps ), // data
            //                           true );
            //            }
            os << "...]";
        }
    } else
        os << "UndefinedType";
}


String Var::type_disp() const {
    std::ostringstream ss;
    write_var_type( ss, type_expr() );
    return ss.str();
}

void Var::write_to_stream( Stream &os ) const {
    write_var( os, type_expr(), expr() );
}

Expr Var::expr() const {
    return data and data->ptr ? data->ptr->expr() : cst( Vec<PI8>() );
}

Expr Var::type_expr() const {
    return type and type->ptr ? type->ptr->expr() : Expr();
}

Var constified( const Var &var ) {
    if ( var.referenced_more_than_one_time() and not var.data->is_const() ) {
        PRINT( var );
        ERROR( "only var that are not referenced more than one time can be fully constified" );
        TODO;
    }
    //
    Var res = var;
    res.data->flags |= PRef::CONST;
    res.flags |= Var::WEAK_CONST;
    return res;
}


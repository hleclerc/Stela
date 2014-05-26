#include "Stela/System/UsualStrings.h"
#include "SysState.h"
#include "Cst.h"
#include "Ip.h"

Ip::Ip() :
    type_SI32  ( STRING_SI32_NUM  , 32 ),
    type_SI64  ( STRING_SI64_NUM  , 64 ),
    type_Void  ( STRING_Void_NUM  ,  0 ),
    type_Bool  ( STRING_Bool_NUM  ,  1 ),
    type_Error ( STRING_Error_NUM ,  0 ),
    // type_RawPtr( STRING_RawPtr_NUM, sizeof( void * ) * 8 ),
    type_ST    ( sizeof( void * ) == 8 ? &type_SI64 : &type_SI32 ),
    sys_state( Ref(), &type_Void, ::sys_state() ) {

    bool f = false, t = true;
    cst_false = cst( 1, (PI8 *)&f );
    cst_true  = cst( 1, (PI8 *)&t );

    cond_stack << cst_true;
}

Var Ip::ret_error( String msg, bool warn, const char *file, int line ) {
    disp_error( msg, warn, file, line );
    return error_var();
}

void Ip::disp_error( String msg, bool warn, const char *file, int line ) {
    std::cerr << error_msg( msg, warn, file, line );
}

ErrorList::Error &Ip::error_msg( String msg, bool warn, const char *file, int line ) {
    ErrorList::Error &res = error_list.add( msg, warn );
    if ( file )
        res.caller_stack.push_back( line, file );

    //    if ( sf )
    //        res.ac( sf_info( sf )->filename, off );
    //    for( Scope *s = sc; s; s = s->caller )
    //        if ( s->instantiated_from_sf )
    //            res.ac( sf_info( s->instantiated_from_sf )->filename, s->instantiated_from_off );
    return res;
}

void Ip::set_cond( Var cond ) {
    set_cond( cond.get_val() );
}

void Ip::set_cond( Expr cond ) {
    cond_stack << simplified( cond );
}

Expr Ip::cur_cond() {
    return simplified( cond_stack.back() );
}

void Ip::pop_cond() {
    cond_stack.pop_back();
}

Var Ip::error_var() {
    return Var( &type_Error );
}

Type *Ip::artificial_type_for_size( int size ) {
    if ( size == 0 ) return &type_Void;
    if ( size == 1 ) return &type_Bool;
    auto iter = art_types.find( size );
    if ( iter != art_types.end() )
        return &iter->second;
    Type &res = art_types[ size ];
    res.name = str_cor.num( "RF" + std::to_string( size ) );
    res._len = size;
    return &res;
}

Ip *ip = 0;

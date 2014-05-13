#include "Stela/System/UsualStrings.h"
#include "SysState.h"
#include "Cst.h"
#include "Ip.h"

Ip::Ip() :
    type_SI32  ( STRING_SI32_NUM   ),
    type_SI64  ( STRING_SI64_NUM   ),
    type_Void  ( STRING_Void_NUM   ),
    type_Bool  ( STRING_Bool_NUM   ),
    type_RawPtr( STRING_RawPtr_NUM ),
    type_ST    ( sizeof( void * ) == 8 ? &type_SI64 : &type_SI32 ),
    sys_state( Ref(), &type_Void, ::sys_state() ) {

    type_SI32  ._size = 32;
    type_SI64  ._size = 64;
    type_Bool  ._size = 1;
    type_Void  ._size = 0;
    type_RawPtr._size = 8 * sizeof( void * );

    cond_stack << cst( true );
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

void Ip::set_cond( const Var &cond ) {
    set_cond( cond.inst );
}

void Ip::set_cond( Ptr<Inst> cond ) {
    cond_stack << simplified( cond );
}

void Ip::pop_cond() {
    cond_stack.pop_back();
}

Ip *ip = 0;

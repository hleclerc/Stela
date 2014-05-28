#include "Stela/System/UsualStrings.h"
#include "CallableData.h"
#include "SysState.h"
#include "Cst.h"
#include "Ip.h"

Ip::Ip() :
    #define DECL_BT( T ) \
        type_##T( STRING_##T##_NUM ),
    #include "DeclBaseClass.h"
    #undef DECL_BT
    type_ST    ( sizeof( void * ) == 8 ? &type_SI64 : &type_SI32 ),
    sys_state( Ref(), &type_Void, ::sys_state() ) {

    // type info
    type_SI64 ._len = 64;
    type_SI32 ._len = 32;
    type_Bool ._len = 1;
    type_Void ._len = 0;
    type_Error._len = 0;

    type_Def  ._len = 8 * sizeof( CallableData );
    type_Class._len = 8 * sizeof( CallableData );

    // std variables
    bool f = false, t = true;
    cst_false = cst( 1, (PI8 *)&f );
    cst_true  = cst( 1, (PI8 *)&t );

    // context
    cond_stack << cst_true;
    off = -1;
    sf = 0;

    // global var
    main_scope = new Scope( 0, "", this );
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

Var Ip::void_var() {
    return Var( &type_Void );
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

Scope::VecNamedVar *Ip::get_static_scope( String path ) {
    return &static_scopes[ path ];
}


void Ip::add_inc_path( String inc_path ) {
    inc_paths.push_back_unique( inc_path );
}

void Ip::import( String file ) {
    main_scope->import( file );
}

SourceFile *Ip::new_sf( String file ) {
    if ( sourcefiles.count( file ) )
        return 0;
    SourceFile *res = &sourcefiles[ file ];
    res->name = file;
    return res;
}

Var Ip::make_Callable( const Vec<Var> &lst, Var self ) {
    TODO;
    return Var();
}


Ip *ip = 0;

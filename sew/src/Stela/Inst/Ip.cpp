#include "../System/BinStreamReader.h"
#include "Class.h"
#include "Type.h"
#include "Cst.h"
#include "Ip.h"

Ip *ip;

Ip::Ip() : main_scope( 0, 0, "", this ), cur_scope( &main_scope ) {
    #define DECL_BT( T ) class_##T = new Class; class_##T->name = STRING_##T##_NUM;
    #include "DeclParmClass.h"
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) type_##T = new Type( class_##T );
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) type_##T->aryth = true;
    #include "DeclArytTypes.h"
    #undef DECL_BT

    type_SI32->_len = 32;
    type_SI64->_len = 64;

    type_ST = sizeof( void * ) == 8 ? type_SI64 : type_SI32;
}

Ip::~Ip() {
    #define DECL_BT( T ) delete type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) delete class_##T;
    #include "DeclParmClass.h"
    #include "DeclBaseClass.h"
    #undef DECL_BT
}

Expr Ip::ret_error( String msg, bool warn, const char *file, int line ) {
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
    for( Scope *s = cur_scope; s; s = s->caller ? s->caller : s->parent )
        if ( s->sf and s->off >= 0 )
            res.ac( s->sf->name.c_str(), s->off );
    return res;
}

Expr Ip::error_var() {
    return cst( type_Error, 0, 0 );
}

Expr Ip::void_var() {
    return cst( type_Void, 0, 0 );
}


Vec<Expr> *Ip::get_static_vars( String path ) {
    return &static_vars[ path ];
}

Expr Ip::reg_var( int name, Expr var ) {
    if ( vars.contains( name ) and not var->is_surdef() )
        return ip->ret_error( "There is already a Expr named '" + ip->str_cor.str( name ) + "' in the current scope" );
    return vars.add( name, var );
}

void Ip::add_inc_path( String inc_path ) {
    inc_paths.push_back_unique( inc_path );
}

SourceFile *Ip::new_sf( String file ) {
    if ( sourcefiles.count( file ) )
        return 0;
    SourceFile *res = &sourcefiles[ file ];
    res->name = file;
    return res;
}

Expr Ip::make_Varargs( Vec<Expr> &v_args, Vec<int> &v_names ) {
    TODO;
    return 0;
}

Expr Ip::make_Callable( Vec<Expr> &lst, Expr self ) {
    TODO;
    return 0;
}

Expr Ip::make_type_var( Type *type ) {
    TODO;
    return 0;
}

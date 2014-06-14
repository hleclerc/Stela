#include "../System/BinStreamReader.h"
#include "ReplBits.h"
#include "Class.h"
#include "Room.h"
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

    #define DECL_BT( T ) type_##T->aryth = true; type_##T->_pod = true;
    #include "DeclArytTypes.h"
    #undef DECL_BT

    type_SI32->_len = 32;
    type_SI64->_len = 64;
    type_Type->_len = 64;

    type_Type->_pod = 1;

    type_ST = sizeof( void * ) == 8 ? type_SI64 : type_SI32;

    sys_state = cst( type_Void );
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

/*
class VarargsItemBeg[ data_type, data_name, next_type ]
    data ~= Ptr[ data_type ]
    next ~= next_type

class VarargsItemEnd
    # void
*/
Type *Ip::make_Varargs_type( const Vec<Type *> &types, const Vec<int> &names, int o ) {
    if ( o == types.size() )
        return type_VarargsItemEnd;
    int n = o < names.size() ? names[ o ] : -1;

    Vec<Expr> lt;
    lt << make_type_var( types[ o ] );
    lt << room( cst( type_SI32, 32, &n ) );
    lt << make_type_var( make_Varargs_type( types, names, o + 1 ) );
    return class_VarargsItemBeg->type_for( lt );
}


Expr Ip::make_Varargs( Vec<Expr> &lst, const Vec<int> &names ) {
    for( int i = 0; i < lst.size(); ++i )
        if ( not lst[ i ] )
            lst.remove( i );

    // type
    Vec<Type *> types;
    for( Expr &v : lst )
        types << v->type();
    Type *type = make_Varargs_type( types, names, 0 );
    type->_len = lst.size() * type_ST->size();
    type->_pod = 1;

    // data
    Expr res( cst( type ) );
    for( int i = 0; i < lst.size(); ++i )
        res = repl_bits( res, i * type_ST->size(), lst[ i ] );
    return room( res );
}

Expr Ip::make_SurdefList( Vec<Expr> &surdefs ) {
    // SurdefList[ surdef_type, parm_type ]
    //   c ~= surdef_type (-> Ptr[Vararg[...]])
    //   p ~= parm_type (-> Void or Ptr[Vararg[...]])
    Expr vs = make_Varargs( surdefs );

    Vec<Expr> lt;
    lt << make_type_var( vs->type() ); // Ptr[VarargItem[...]]
    lt << make_type_var( type_Void );
    Type *type = class_SurdefList->type_for( lt );
    type->_len = type_ST->size();
    type->_pod = 1;

    // data
    Expr res = cst( type );
    res = repl_bits( res, 0, vs );
    return room( res );
}

Expr Ip::make_type_var( Type *type ) {
    SI64 ptr = (SI64)type;
    return room( cst( type_Type, 64, &ptr ) );
}


Type *Ip::type_from_type_var( Expr var ) {
    SI64 ptr;
    Expr e = var->get();
    if ( e->get_val( type_Type, &ptr ) )
        return reinterpret_cast<Type *>( ST( ptr ) );
    return type_Error;
}

Type *Ip::ptr_for( Type *type ) {
    auto it = ptr_map.find( type );
    if ( it != ptr_map.end() )
        return it->second;

    Vec<Expr> tl = make_type_var( type );
    Type *res = class_Ptr->type_for( tl );
    res->_len = type_ST->size();
    res->_pod = 1;

    ptr_map[ type ] = res;
    return res;
}

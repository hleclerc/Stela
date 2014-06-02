#include "Stela/System/UsualStrings.h"
#include "CallableData.h"
#include "SysState.h"
#include "Cst.h"
#include "Def.h"
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

    type_Type ._len = 64; // ptr to Type

    type_Def  ._len = 8 * sizeof( CallableData );
    type_Class._len = 8 * sizeof( CallableData );

    #define DECL_BT( T ) type_##T.orig = &class_##T; class_##T.types << &type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    // class
    #define DECL_BT( T ) class_##T.name = STRING_##T##_NUM;
    #include "DeclBaseClass.h"
    #include "DeclParmClass.h"
    #undef DECL_BT

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

    if ( sf and off >= 0 )
        res.ac( sf->name.c_str(), off );
    for( int i = sf_stack.size() - 1; i >= 0; --i )
        if ( sf_stack[ i ].sf and sf_stack[ i ].off >= 0 )
            res.ac( sf_stack[ i ].sf->name.c_str(), sf_stack[ i ].off );
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

int Ip::read_nstring( BinStreamReader &bin ) {
    return sf->cor_str[ bin.read_positive_integer() ];
}

Var Ip::make_type_var( Type *type ) {
    SI64 ptr = (SI64)type;
    return Var( &type_Type, cst( 64, (PI8 *)&ptr ) );
}

Type *Ip::type_from_type_var( Var var ) {
    if ( var.type != &ip->type_Type )
        return 0;
    SI64 p;
    Expr v = var.get_val();
    if ( not v->get_val( p ) )
        return 0;
    return reinterpret_cast<Type *>( ST( p ) );
}

bool Ip::ext_method( Var m ) {
    if ( m.type != &type_Def )
        return false;
    SI64 p;
    if ( not m.get_val()->get_val( p ) ) {
        disp_error( "expecting a known value" );
        return false;
    }
    return reinterpret_cast<Def *>( ST( p ) )->self_as_arg();
}

void Ip::push_sf( SourceFile *nsf, const char *reason ) {
    sf_stack << CS{ sf, off, reason };
    sf  = nsf;
    off = -1;
}

void Ip::pop_sf() {
    sf  = sf_stack.back().sf;
    off = sf_stack.back().off;
    sf_stack.pop_back();
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
        return &type_VarargsItemEnd;
    int n = o < names.size() ? names[ o ] : -1;

    Vec<Var> lt;
    lt << make_type_var( types[ o ] );
    lt << Var( &type_SI32, cst( 32, (PI8 *)&n ) );
    lt << make_type_var( make_Varargs_type( types, names, o + 1 ) );
    return class_VarargsItemBeg.type_for( lt );
}

Var Ip::make_Varargs( Vec<Var> lst, const Vec<int> &names ) {
    // type
    Vec<Type *> types;
    for( const Var &v : lst )
        types << v.type;
    Type *type = make_Varargs_type( types, names, 0 );
    type->_len = lst.size() * type_ST->size();

    // data
    Var res( type );
    for( int i = 0; i < lst.size(); ++i )
        res.set_val( i * type_ST->size(), lst[ i ].ptr() );
    return res;
}

Var Ip::make_Callable( Vec<Var> lst, Var self ) {
    // template arguments:
    // -> varargs with unnamed arguments
    // -> self_type
    // -> parm_type
    // attributes:
    // -> self_ptr (ST)
    // -> parm_ptr (ST)
    Vec<Var> lt;
    lt << make_Varargs( lst );
    if ( self.defined() )
        lt << make_type_var( self.type );
    else
        lt << make_type_var( &type_Void );
    lt << make_type_var( &type_Void );
    Type *type = class_Callable.type_for( lt );
    type->_len = 2 * type_ST->size();

    // data
    SI64 d = 0;
    Var res( type );
    if ( self.defined() )
        res.set_val( 0 * type_ST->size(), self.ptr() );
    else
        res.set_val( 0 * type_ST->size(), type_ST, cst( type_ST->size(), (PI8 *)&d ) );
    res.set_val( 1 * type_ST->size(), type_ST, cst( type_ST->size(), (PI8 *)&d ) );
    return res;
}


Ip *ip = 0;

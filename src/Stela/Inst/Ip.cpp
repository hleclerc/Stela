#include "../System/BinStreamReader.h"
#include "ReplBits.h"
#include "Class.h"
#include "Slice.h"
#include "Room.h"
#include "Type.h"
#include "Cst.h"
#include "Ip.h"
#include "Op.h"

Ip *ip;

Ip::Ip() : main_scope( 0, 0, "", this ), cur_scope( &main_scope ), cur_ip_snapshot( 0 ) {
    #define DECL_BT( T ) class_##T = new Class; class_##T->name = STRING_##T##_NUM;
    #include "DeclParmClass.h"
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) type_##T = new Type( class_##T ); class_##T->types << type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) type_##T->aryth = true; type_##T->_pod = true; type_##T->_len = SizeofIf<T,true>::val; type_##T->_ali = type_##T->_len;
    #include "DeclArytTypes.h"
    #undef DECL_BT

    type_Type ->_len = 64; type_Type ->_ali = 32; type_Type ->_pod = 1;
    type_Void ->_len =  0; type_Void ->_ali =  1; type_Void ->_pod = 1;
    type_Error->_len =  0; type_Error->_ali =  1; type_Error->_pod = 1;
    type_Def  ->_len = 64; type_Def  ->_ali = 32; type_Def  ->_pod = 1;
    type_Class->_len = 64; type_Class->_ali = 32; type_Class->_pod = 1;

    type_ST  = sizeof( void * ) == 8 ? type_SI64 : type_SI32;
    ptr_size = 8 * sizeof( void * );

    sys_state = room( cst( type_ST ) );


    Vec<Expr> vs( make_type_var( type_SI32 ) );
    type_Ptr_SI32 = class_Ptr->type_for( vs );
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


NamedVarList *Ip::get_static_vars( String path ) {
    return &static_vars[ path ];
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
    type->_len = lst.size() * ptr_size;
    type->_ali = 32;
    type->_pod = 1;

    // data
    Expr res( cst( type ) );
    for( int i = 0; i < lst.size(); ++i )
        res = repl_bits( res, i * ptr_size, lst[ i ] );
    return room( res );
}

Expr Ip::make_SurdefList( Vec<Expr> &surdefs, Expr self ) {
    // SurdefList[ surdef_type, parm_type, self_type ]
    //   c ~= surdef_type (-> Ptr[VarargBeg[...]] with )
    //   p ~= parm_type (-> Void or Ptr[Vararg[...]])
    //   s ~= self_type (-> Void or Ptr[Vararg[...]])
    Expr vs = make_Varargs( surdefs );

    Vec<Expr> lt;
    lt << make_type_var( vs->type() ); // Ptr[VarargItem[...]]
    lt << make_type_var( type_Void );
    if ( self )
        lt << make_type_var( self->type() );
    else
        lt << make_type_var( type_Void );
    Type *type = class_SurdefList->type_for( lt );
    type->_len = ptr_size + bool( self ) * ptr_size;
    type->_ali = 32;
    type->_pod = 1;

    // data
    Expr res = cst( type );
    res = repl_bits( res, 0, vs );
    if ( self )
        res = repl_bits( res, ptr_size, self );
    return room( res );
}

Expr Ip::make_type_var( Type *type ) {
    SI64 ptr = (SI64)type;
    return room( cst( type_Type, 64, &ptr ) );
}


Type *Ip::type_from_type_var( Expr var ) {
    // if Type
    SI64 ptr;
    Expr e = var->get();
    if ( e->get_val( type_Type, &ptr ) )
        return reinterpret_cast<Type *>( ST( ptr ) );
    //
    Expr p = ip->main_scope.apply( var, 0, 0, 0, 0, 0, Scope::APPLY_MODE_PARTIAL_INST );
    return p->ptype();
}

Type *Ip::ptr_for( Type *type ) {
    auto it = ptr_map.find( type );
    if ( it != ptr_map.end() )
        return it->second;

    if ( type == type_Error )
        return type;

    Vec<Expr> tl = make_type_var( type );
    Type *res = class_Ptr->type_for( tl );
    res->_len = ptr_size;
    res->_ali = 32;
    res->_pod = 1;

    ptr_map[ type ] = res;
    return res;
}

void Ip::get_args_in_varargs( Vec<Expr> &args, Vec<int> &names, Expr lst ) {
    int o = 0, tn;
    for ( Type *vt = lst->type(); vt->orig == ip->class_VarargsItemBeg; vt = ip->type_from_type_var( vt->parameters[ 2 ] ), o += ip->ptr_size ) {
        args << slice( ip->type_from_type_var( vt->parameters[ 0 ] ), lst, o );
        if ( not vt->parameters[ 1 ]->get()->get_val( type_SI32, &tn ) )
            return disp_error( "expecting a known SI32 as second arg of a varargs" );
        if ( tn >= 0 )
            names << tn;
    }
}

bool Ip::is_integer( Type *type ) {
    return type == type_Bool or
           type == type_PI8  or
           type == type_PI16 or
           type == type_PI32 or
           type == type_PI64 or
           type == type_SI8  or
           type == type_SI16 or
           type == type_SI32 or
           type == type_SI64;

}

Vec<Expr> Ip::get_args_in_varargs( Expr lst ) {
    Vec<Expr> args;
    Vec<int> names;
    get_args_in_varargs( args, names, lst );
    return args;
}

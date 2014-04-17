#include "../System/ReadFile.h"
#include "../System/ToDel.h"
#include "../Met/IrWriter.h"
#include "../Met/Lexer.h"

#include "Interpreter.h"
#include "ClassInfo.h"
#include "TypeInfo.h"
#include "DefInfo.h"
#include "RefExpr.h"
#include "Scope.h"

#include "../Ir/Numbers.h"

#include "../Inst/PointerOn.h"
#include "../Inst/BaseType.h"
#include "../Inst/Concat.h"
#include "../Inst/Slice.h"
#include "../Inst/ValAt.h"
#include "../Inst/Arch.h"
#include "../Inst/Cst.h"

Interpreter *ip = 0;


Interpreter::Interpreter( ErrorList &error_list ) :

    #define DECL_BT( T ) type_##T( &type_Type ),
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) class_##T( &type_Class ),
    #include "DeclBaseClass.h"
    #include "DeclParmClass.h"
    #undef DECL_BT

    error_var( &type_Error ),
    void_var( &type_Void ),
    error_list( error_list ) {


    main_scope = 0;

    bt_ST   = sizeof( ST ) == 8 ? bt_SI64    : bt_SI32;
    type_ST = sizeof( ST ) == 8 ? &type_SI64 : &type_SI32;

    // constify
    error_var.data->flags = PRef::CONST;
    void_var.data->flags = PRef::CONST;

    #define DECL_BT( T ) \
        type_##T.data->flags = PRef::CONST;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) class_##T.data->flags = PRef::CONST;
    #include "DeclBaseClass.h"
    #include "DeclParmClass.h"
    #undef DECL_BT
}

Interpreter::~Interpreter(){
    delete main_scope;

    for( auto p: class_info_map )
        delete p.second;
    for( auto p: def_info_map )
        delete p.second;
    for( int i = obj_to_delete.size() - 1; i >= 0; --i )
        delete obj_to_delete[ i ];
}

void Interpreter::import( String filename ) {
    if ( already_imported( filename ) )
        return;

    // -> source data
    ReadFile r( filename.c_str() );
    if ( not r )
        return disp_error( "Impossible to open " + filename );

    // -> lexical data
    Lexer l( error_list );
    l.parse( r.data, filename.c_str() );
    if ( error_list )
        return;

    // -> binary stream
    IrWriter t( error_list );
    t.parse( l.root() );
    if ( error_list )
        return;

    // -> make a new SourceFile
    ST bin_size = t.size_of_binary_data();
    Vec<PI8> data( Size(), sizeof( SI32 ) + bin_size + sizeof( SI32 ) + filename.size() + 1 );
    reinterpret_cast<SI32 &>( data[ 0 ] ) = bin_size;
    t.copy_binary_data_to( data.ptr() + sizeof( SI32 ) );
    reinterpret_cast<SI32 &>( data[ sizeof( int ) + bin_size ] ) = filename.size();
    memcpy( data.ptr() + sizeof( SI32 ) + bin_size + sizeof( SI32 ), filename.c_str(), filename.size() + 1 );

    const Expr &sf = *sourcefiles.push_back( pointer_on( cst( data.ptr(), 0, 8 * data.size() ) ) );

    // -> virtual machine
    if ( not main_scope )
        main_scope = new Scope( 0 );
    main_scope->parse( sf, sf_info( sf )->tok_data );
}

Vec<ConstPtr<Inst> > Interpreter::get_outputs() {
    Vec<ConstPtr<Inst> > res;
    if ( main_scope->sys_state and not main_scope->sys_state.cst_data() )
        res << main_scope->sys_state.expr().inst;
    return res;
}

void Interpreter::add_inc_path( String path ) {
    inc_paths << path;
}

ErrorList::Error &Interpreter::make_error( String msg, const Expr &sf, int off, Scope *sc, bool warn ) {
    ErrorList::Error &res = error_list.add( msg );
    if ( sf )
        res.ac( sf_info( sf )->filename, off );
    for( Scope *s = sc; s; s = s->caller )
        if ( s->instantiated_from_sf )
            res.ac( sf_info( s->instantiated_from_sf )->filename, s->instantiated_from_off );

    return res;
}

void Interpreter::disp_error( String msg, const Expr &sf, int off, Scope *sc, bool warn ) {
    std::cerr << make_error( msg, sf, off, sc, warn );
}

bool Interpreter::already_imported( String filename ) {
    for( int i = 0; i < sourcefiles.size(); ++i )
        if ( sf_info( sourcefiles[ i ] )->filename == filename )
            return true;
    return false;
}

SfInfo *Interpreter::sf_info( const Expr &sf ) {
    auto iter = sf_info_map.find( sf );
    if ( iter != sf_info_map.end() )
        return &iter->second;

    // -> new SfInfo
    SfInfo &res = sf_info_map[ sf ];

    // get a pointer to nstr table and tok data (@see import for the data format)
    const PI8 *ptr = sf.vat_data();

    // nstr_cor
    BinStreamReader bsr( ptr + sizeof( SI32 ) );
    while ( true ) {
        ST off = bsr.read_positive_integer();
        if ( not off )
            break;
        res.nstr_cor << glob_nstr_cor.num( String( bsr.ptr, bsr.ptr + off ) );
        bsr.ptr += off;
    }

    // tok_data
    res.tok_data = bsr.ptr;

    // filename
    SI32 bin_len = *reinterpret_cast<const SI32 *>( ptr );
    res.filename = reinterpret_cast<const char *>( ptr + sizeof( SI32 ) + bin_len + sizeof( SI32 ) );

    return &res;
}

int Interpreter::glo_nstr( const Expr &sf, int n ) {
    SfInfo *si = sf_info( sf );
    ASSERT( n < si->nstr_cor.size(), "bad nstr" );
    return si->nstr_cor[ n ];
}

Var Interpreter::ext_method( const Var &var ) {
    if ( isa_Def( var ) )
        if ( DefInfo *d = def_info( pointer_on( var.expr() ) ) )
            if ( d->self_as_arg() )
                return var;
    return Var();
}

Var Interpreter::make_Callable( Vec<Var> lst, Var self ) {
    // remove doubles
    for( int i = 0; i < lst.size(); ++i )
        for( int j = i + 1; j < lst.size(); ++j )
            if ( lst[ i ].expr() == lst[ j ].expr() )
                lst.remove_unordered( j-- );

    //
    Expr surdef_list_data = cst( SI32( lst.size() ) );
    for( int i = 0; i < lst.size(); ++i )
        surdef_list_data = concat( surdef_list_data, pointer_on( lst[ i ].expr() ) );
    Var surdef_list( &type_SurdefList, surdef_list_data );

    //
    Var self_type = type_of( self ); // returns void if self is not defined

    // -> Callable[ surdef_list, self_type, parm_type ]
    Var *parms[ 3 ];
    parms[ 0 ] = &surdef_list;
    parms[ 1 ] = &self_type;
    parms[ 2 ] = &void_var;

    Var *callable_type = type_for( class_info( class_Callable ), parms );
    return Var( callable_type, self ? pointer_on( self.expr() ) : cst() ).add_ref( 0, self );
}

Var Interpreter::update_Callable( Var callable, Var varargs ) {
    // -> new Callable[ surdef_list, self_type, parm_type ]
    TypeInfo *type = type_info( callable.type_expr() );
    if ( isa_Void( type->parameters[ 2 ] ) ) {
        // -> no previously defined parms
        Expr n_data = concat( callable.expr(), pointer_on( varargs.expr() ) );
        Var varargs_type = type_of( varargs );


        // -> Callable[ surdef_list, self_type, parm_type ]
        Var *parms[ 3 ];
        parms[ 0 ] = &type->parameters[ 0 ];
        parms[ 1 ] = &type->parameters[ 1 ];
        parms[ 2 ] = &varargs_type;

        Var res( type_for( class_info( class_Callable ), parms ), n_data );
        res.add_ref_from( callable );
        res.add_ref( callable.expr().size_in_bits(), varargs );
        return res;
    }
    //
    PRINT( type->parameters[ 2 ] );
    TODO;
    return error_var;
}

Var Interpreter::type_of( const Var &var ) const {
    if ( not var.type )
        return void_var;

    Var res;
    res.data  = var.type;
    res.type  = type_Type.data;
    res.flags = Var::WEAK_CONST;
    return res;
}

TypeInfo *Interpreter::type_info( const Expr &type ) {
    auto iter = type_info_map.find( type );
    ASSERT( iter != type_info_map.end(), "not a registered type var" );

    TypeInfo *res = iter->second;
    res->parse_if_necessary();
    return res;
}

CallableInfo *Interpreter::callable_info( const Expr &callable_ptr ) {
    if ( CallableInfo *ci = class_info( callable_ptr, false ) ) return ci;
    if ( CallableInfo *ci =   def_info( callable_ptr, false ) ) return ci;
    return 0;
}

ClassInfo *Interpreter::class_info( const Expr &class_ptr, bool crea, Ptr<VarTable> sn ) {
    auto iter = class_info_map.find( class_ptr );
    if ( iter != class_info_map.end() )
        return iter->second;
    if ( not crea )
        return 0;

    // make a new ClassInfo from class_ptr expr
    SI32 a = arch->ptr_size, b = a + 32, c = b + 32, src_off = 0, bin_off = 0;
    Expr sf = val_at( class_ptr, 0, a );
    val_at( class_ptr, a, b ).get_val( bin_off );
    val_at( class_ptr, b, c ).get_val( src_off );

    ClassInfo *res = new ClassInfo( sf, src_off, sf.vat_data() + bin_off, class_ptr, sn );
    class_info_map[ class_ptr ] = res;
    return res;
}

DefInfo *Interpreter::def_info( const Expr &def_ptr, bool crea, Ptr<VarTable> sn ) {
    auto iter = def_info_map.find( def_ptr );
    if ( iter != def_info_map.end() )
        return iter->second;
    if ( not crea )
        return 0;

    // make a new DefInfo from def_ptr expr
    SI32 a = arch->ptr_size, b = a + 32, c = b + 32, src_off = 0, bin_off = 0;
    Expr sf = val_at( def_ptr, 0, a );
    val_at( def_ptr, a, b ).get_val( bin_off );
    val_at( def_ptr, b, c ).get_val( src_off );

    DefInfo *res = new DefInfo( sf, src_off, sf.vat_data() + bin_off, sn );
    def_info_map[ def_ptr ] = res;
    return res;
}


ClassInfo *Interpreter::class_info( const Var &class_var ) {
    return class_info( pointer_on( class_var.expr() ) );
}


Var *Interpreter::type_for( ClassInfo *class_info ) {
    ASSERT( class_info->nb_arg_vals() == 0, "bad nb args" );
    return type_for( class_info, (Var **)0 );
}

Var *Interpreter::type_for( ClassInfo *class_info, Var *parm_0 ) {
    ASSERT( class_info->nb_arg_vals() == 1, "bad nb args" );
    return type_for( class_info, &parm_0 );
}

Var Interpreter::_get_type_var( ClassInfo *class_info ) {
    #define DECL_BT( T ) if ( class_info->name == STRING_##T##_NUM ) return type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT
    return Var( &type_Type );
}

Var Interpreter::copied_or_constified( const Var &var ) {
    // need to make a copy ?
    if ( var.referenced_more_than_one_time() and not var.data->is_const() )
        return constified( main_scope->copy( var, Expr(), 0 ) );
    // else, add const flags
    Var res = var;
    res.data->flags |= PRef::CONST;
    res.flags |= Var::WEAK_CONST;
    return res;
}

Var *Interpreter::type_for( ClassInfo *class_info, Var **parm_l ) {
    for( TypeInfo *t = class_info->last; t; t = t->prev ) {
        // ASSERT( t->parameters.size() == parm_s, "parameter lists not of the same size" );
        for( int i = 0; ; ++i ) {
            if ( i == t->parameters.size() )
                return &t->var;
            if ( not equal( t->parameters[ i ], *parm_l[ i ] ) )
                break;
        }
    }

    // -> new TypeInfo
    TypeInfo *res = new TypeInfo( class_info );
    for( int i = 0; i < class_info->arg_names.size(); ++i )
        res->parameters << copied_or_constified( *parm_l[ i ] );

    // Data:
    //  - ptr to parent class
    //  - type_ptr + [parm type, parm ref]* nb parameters as defined in parent class
    Expr re = class_info->class_ptr;
    for( int i = 0; i < res->parameters.size(); ++i ) {
        ASSERT( parm_l[ i ]->type, "Type not defined" );
        re = concat( re, parm_l[ i ]->type->expr(), pointer_on( res->parameters[ i ].expr() ) );
    }

    type_info_map[ re ] = res;

    res->var = constified( _get_type_var( class_info ) );
    res->var.data->ptr = new RefExpr( re );
    for( int i = 0; i < res->parameters.size(); ++i )
        res->var.add_ref( ( 2 + 2 * i ) * arch->ptr_size, res->parameters[ i ] );

    res->prev = class_info->last;
    class_info->last = res;
    return &res->var;
}

bool Interpreter::equal( Var a, Var b ) {
    if ( isa_Type( a ) and isa_Type( b ) )
        return a.expr() == b.expr();
    if ( isa_SurdefList( a ) and isa_SurdefList( b ) )
        return a.expr() == b.expr();
    if ( isa_Callable( a ) and isa_Callable( b ) )
        return a.expr() == b.expr();
    if ( isa_SI32( a ) and isa_SI32( b ) )
        return a.expr() == b.expr();
    if ( isa_Void( a ) )
        return isa_Void( b );
    if ( isa_Void( b ) )
        return isa_Void( a );
    PRINT( a );
    PRINT( b );
    TODO;
    return false;
}

Expr Interpreter::cst_ptr( SI64 val ) {
    if ( arch->ptr_size == 32 )
        return cst( SI32( val ) );
    return cst( val );
}

Var Interpreter::make_varargs_var( const Vec<Var> &uv_args, const Vec<Var> &nv_args, const Vec<int> &nv_name ) {
    Var *type = _make_varargs_type( uv_args, nv_args, nv_name, 0 );

    Expr data = cst();
    for( int i = 0; i < uv_args.size(); ++i )
        data = concat( data, pointer_on( uv_args[ i ].expr() ) );
    for( int i = 0; i < nv_args.size(); ++i )
        data = concat( data, pointer_on( nv_args[ i ].expr() ) );

    Var res( type, data );
    for( int i = 0, off = 0; i < uv_args.size(); off += bt_ST->size_in_bits(), ++i )
        res.add_ref( off, uv_args[ i ] );
    for( int i = 0, off = 0; i < nv_args.size(); off += bt_ST->size_in_bits(), ++i )
        res.add_ref( off, nv_args[ i ] );
    return res;
}

Var *Interpreter::_make_varargs_type( const Vec<Var> &uv_args, const Vec<Var> &nv_args, const Vec<int> &nv_name, int off ) {
    // class VarargsItemEnd
    if ( off >= uv_args.size() + nv_args.size() )
        return &type_VarargsItemEnd;

    // class VarargsItemBeg[ data_type, data_name, next_type ]
    Var data_type = type_of( off >= uv_args.size() ? nv_args[ off - uv_args.size() ] : uv_args[ off ] );
    Var data_name( &type_SI32, cst( off >= uv_args.size() ? nv_name[ off - uv_args.size() ] : -1 ) );

    Var *parms[ 3 ];
    parms[ 0 ] = &data_type;
    parms[ 1 ] = &data_name;
    parms[ 2 ] = _make_varargs_type( uv_args, nv_args, nv_name, off + 1 );
    return type_for( class_info( class_VarargsItemBeg ), parms );
}

void Interpreter::_update_base_type_from_class_expr( Var type, Expr class_expr ) {
    Expr type_expr = pointer_on( class_expr ); // no parameters.
    type.data->ptr = new RefExpr( type_expr );
    type_for( class_info( pointer_on( class_expr ) ) ); ///< fill type_info_map
}

bool Interpreter::isa_ptr_int( const Var &var ) const {
    if ( arch->ptr_size == 32 )
        return isa_SI32( var ) or isa_PI32( var );
    return isa_SI64( var ) or isa_PI64( var );
}

bool Interpreter::is_of_class( const Var &var, const Var &class_ ) const {
    Expr class_ptr = slice( var.type->ptr->expr(), 0, arch->ptr_size );
    return val_at( class_ptr, arch->ptr_size + 2 * 32 ) == class_.expr();
}

int Interpreter::to_bool( Var val, const Expr &sf, const PI8 *tok ) {
    if ( not isa_Bool( val ) )
        TODO;
    bool res;
    return conv( res, val ) ? res : -1;
}

#define DECL_BT( T ) bool Interpreter::isa_##T( const Var &var ) const { return is_of_class( var, class_##T ); }
#include "DeclParmClass.h"
#undef DECL_BT

const BaseType *Interpreter::bt_of( const Var &var ) const {
    #define DECL_BT( T ) if ( isa_##T( var ) ) return bt_##T;
    #include "../Inst/DeclArytTypes.h"
    #undef DECL_BT
    return 0;
}

Var *Interpreter::type_for( const BaseType *bt ) {
    #define DECL_BT( T ) if ( bt == bt_##T ) return &type_##T;
    #include "../Inst/DeclArytTypes.h"
    #undef DECL_BT
    return 0;
}

bool Interpreter::isa_POD( const Var &var ) const {
    #define DECL_BT( T ) if ( isa_##T( var ) ) return true;
    #include "../Inst/DeclArytTypes.h"
    #undef DECL_BT
    return false;
}


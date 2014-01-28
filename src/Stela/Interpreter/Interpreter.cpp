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

    bt_ST = sizeof( ST ) == 8 ? bt_SI64 : bt_SI32;

    // constify
    error_var.data->flags = PRef::CONST;
    void_var.data->flags = PRef::CONST;

    #define DECL_BT( T ) type_##T.data->flags = PRef::CONST;
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

    Expr *sf = sourcefiles.push_back( pointer_on( cst( data.ptr(), 0, 8 * data.size() ) ) );

    // -> virtual machine
    if ( not main_scope )
        main_scope = new Scope( 0 );
    main_scope->parse( sf, sf_info( sf )->tok_data );
}

Vec<ConstPtr<Inst> > Interpreter::get_outputs() {
    Vec<ConstPtr<Inst> > res;
    if ( main_scope->sys_state )
        res << main_scope->sys_state.expr().inst;
    return res;
}

void Interpreter::add_inc_path( String path ) {
    inc_paths << path;
}

ErrorList::Error &Interpreter::make_error( String msg, const Expr *sf, int off, Scope *sc, bool warn ) {
    ErrorList::Error &res = error_list.add( msg );
    if ( sf )
        res.ac( sf_info( sf )->filename, off );
    for( Scope *s = sc; s; s = s->caller )
        if ( s->instantiated_from_sf )
            res.ac( sf_info( s->instantiated_from_sf )->filename, s->instantiated_from_off );

    return res;
}

void Interpreter::disp_error( String msg, const Expr *sf, int off, Scope *sc, bool warn ) {
    std::cerr << make_error( msg, sf, off, sc, warn );
}

bool Interpreter::already_imported( String filename ) {
    for( int i = 0; i < sourcefiles.size(); ++i )
        if ( sf_info( &sourcefiles[ i ] )->filename == filename )
            return true;
    return false;
}

SfInfo *Interpreter::sf_info( const Expr *sf ) {
    auto iter = sf_info_map.find( *sf );
    if ( iter != sf_info_map.end() )
        return &iter->second;

    // -> new SfInfo
    SfInfo &res = sf_info_map[ *sf ];

    // get a pointer to nstr table and tok data (@see import for the data format)
    const PI8 *ptr = sf->vat_data();

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

int Interpreter::glo_nstr( const Expr *sf, int n ) {
    SfInfo *si = sf_info( sf );
    ASSERT( n < si->nstr_cor.size(), "bad nstr" );
    return si->nstr_cor[ n ];
}


Var Interpreter::type_of( const Var &var ) const {
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
    if ( not res->parsed ) {
        Scope scope( main_scope, 0 );
        scope.parse( res->orig->block.sf, res->orig->block.tok );

        PRINT( res->orig->class_ptr );
        TODO;
    }
    return res;
}

CallableInfo *Interpreter::callable_info( const Expr &callable_ptr ) {
    if ( CallableInfo *ci = class_info( callable_ptr ) ) return ci;
    if ( CallableInfo *ci =   def_info( callable_ptr ) ) return ci;
    return 0;
}

ClassInfo *Interpreter::class_info( const Expr &class_ptr ) {
    auto iter = class_info_map.find( class_ptr );
    if ( iter != class_info_map.end() )
        return iter->second;

    // make a new ClassInfo from class_ptr expr
    SI32 a = arch->ptr_size, b = a + 32, c = b + 32, src_off = 0, bin_off = 0;
    Expr sf = val_at( class_ptr, 0, a );
    val_at( class_ptr, a, b ).basic_conv( bin_off );
    val_at( class_ptr, b, c ).basic_conv( src_off );

    ClassInfo *res = new ClassInfo( &sf, src_off, sf.vat_data() + bin_off, class_ptr );
    class_info_map[ class_ptr ] = res;
    return res;
}

DefInfo *Interpreter::def_info( const Expr &def_ptr ) {
    auto iter = def_info_map.find( def_ptr );
    if ( iter != def_info_map.end() )
        return iter->second;
    TODO;
    return 0;
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
        res->parameters << constified( *parm_l[ i ] );

    // Data:
    //  - ptr to parent class
    //  - ref on parameter [* nb parameters as defined in parent class]
    Expr re = class_info->class_ptr;
    for( int i = 0; i < res->parameters.size(); ++i )
        re = concat( re, ref_expr_on( res->parameters[ i ] ) );

    type_info_map[ re ] = res;

    res->var = Var( &type_Type, re );
    res->prev = class_info->last;
    class_info->last = res;
    return &res->var;
}

bool Interpreter::equal( Var a, Var b ) {
    if ( isa_Type( a ) and isa_Type( b ) )
        return a.expr() == b.expr();
    TODO;
    return false;
}

Expr Interpreter::cst_ptr( SI64 val ) {
    if ( arch->ptr_size == 32 )
        return cst( SI32( val ) );
    return cst( val );
}

void Interpreter::_update_base_type_from_class_expr( Var type, Expr class_expr ) {
    Expr type_expr = pointer_on( class_expr ); // no parameters.
    type.data->ptr = new RefExpr( type_expr );
}

Expr Interpreter::ref_expr_on( const Var &var ) {
    VarRef &res = var_refs[ var.data.ptr() ];
    res.var = var;
    ++res.cpt;

    return pointer_on( var.expr() );
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

#define DECL_BT( T ) bool Interpreter::isa_##T( const Var &var ) const { return is_of_class( var, class_##T ); }
#include "DeclParmClass.h"
#undef DECL_BT


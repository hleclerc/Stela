#include "../System/BinStreamReader.h"
#include "../System/ReadFile.h"
#include "../Met/IrWriter.h"
#include "../Met/Lexer.h"
#include "Sourcefile.h"
#include "Scope.h"
#include "Cst.h"
#include "Ip.h"

#include "../Ir/AssignFlags.h"
#include "../Ir/Numbers.h"

Scope::Scope( Scope *parent, String name, Ip *ip ) : ip( ip ) {
    if ( parent )
        path = parent->path + "/";
    path += name;

    static_scope = ip->get_static_scope( path );
    do_not_execute_anything = false;
}

void Scope::import( String file ) {
    SourceFile *sf = ip->new_sf( file );
    if ( not sf )
        return;

    // -> source data
    ReadFile r( file.c_str() );
    if ( not r )
        return ip->disp_error( "Impossible to open " + file );

    // -> lexical data
    Lexer l( ip->error_list );
    l.parse( r.data, file.c_str() );
    if ( ip->error_list )
        return;

    IrWriter t( ip->error_list );
    t.parse( l.root() );
    if ( ip->error_list )
        return;

    // -> fill sf->tok_data
    sf->tok_data.resize( t.size_of_binary_data() );
    t.copy_binary_data_to( sf->tok_data.ptr() );

    // -> cor str
    BinStreamReader bin( sf->tok_data.ptr() );
    while ( true ) {
        int s = bin.read_positive_integer();
        if ( not s )
            break;
        sf->cor_str << ip->str_cor.num( String( bin.ptr, bin.ptr + s ) );
        bin.ptr += s;
    }

    // -> instructions
    ip->sf = sf;
    parse( bin.ptr );
}

Var Scope::parse( const PI8 *tok ) {
    if ( tok == 0 or do_not_execute_anything )
        return ip->error_var();

    BinStreamReader bin( tok );
    PI8 tva = bin.get<PI8>(); ///< token type
    ip->off = bin.read_positive_integer(); ///< offset in sourcefile

    switch ( tva ) {
        #define DECL_IR_TOK( N ) case IR_TOK_##N: return parse_##N( bin );
        #include "../Ir/Decl.h"
        #undef DECL_IR_TOK
        default: return ip->ret_error( "Unknown token type" );
    }
}

Var Scope::parse_BLOCK( BinStreamReader bin ) {
    Var res;
    while ( const PI8 *tok = bin.read_offset() )
        res = parse( tok );
    return res;
}

Var Scope::parse_DEF( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_CLASS( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_RETURN( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_APPLY( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_SELECT( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_CHBEBA( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_SI32( BinStreamReader bin ) {
    SI32 data = bin.read_positive_integer();
    return Var( &ip->type_SI32, cst( 32, (PI8 *)&data ) );
}
Var Scope::parse_PI32( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_SI64( BinStreamReader bin ) {
    SI64 data = bin.read_positive_integer();
    PRINT( data );
    return Var( &ip->type_SI64, cst( 64, (PI8 *)&data ) );
}
Var Scope::parse_PI64( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_PTR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_STRING( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_VAR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_ASSIGN( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_REASSIGN( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_GET_ATTR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_GET_ATTR_PTR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_GET_ATTR_ASK( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_GET_ATTR_PTR_ASK( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_GET_ATTR_PA( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_IF( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_WHILE( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_BREAK( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_CONTINUE( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_FALSE( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_TRUE( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_VOID( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_SELF( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_THIS( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_FOR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_IMPORT( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_NEW( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_LIST( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_LAMBDA( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_NULL_REF( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_AND( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_OR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}

Var Scope::parse_info( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_disp( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_rand( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_syscall( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    Vec<Var> inp( Rese(), n );
    for( int i = 0; i < n; ++i )
        inp << parse( bin.read_offset() );

    return syscall( inp );
}
Var Scope::parse_set_base_size_and_alig( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_set_RawRef_dependancy( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_reassign_rec( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_assign_rec( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_set_ptr_val( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_select_SurdefList( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_ptr_size( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_ptr_alig( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_size_of( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_alig_of( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_typeof( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_address( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_get_slice( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_pointed_value( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_pointer_on( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_block_exec( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_get_argc( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_get_argv( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_apply_LambdaFunc( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_inst_of( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}

#define DECL_IR_TOK( N ) Var Scope::parse_##N( BinStreamReader bin ) { std::cout << #N << std::endl; return ip->error_var(); }
#include "../Ir/Decl_UnaryOperations.h"
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

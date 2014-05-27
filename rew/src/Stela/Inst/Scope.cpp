#include "../System/BinStreamReader.h"
#include "../System/ReadFile.h"
#include "../Met/IrWriter.h"
#include "../Met/Lexer.h"
#include "Sourcefile.h"
#include "Symbol.h"
#include "Select.h"
#include "Scope.h"
#include "Cst.h"
#include "Op.h"
#include "Ip.h"

#include "../Ir/AssignFlags.h"
#include "../Ir/Numbers.h"

Scope::Scope( Scope *parent, String name, Ip *_ip ) : ip( _ip ? _ip : ::ip ), parent( parent ) {

    if ( parent )
        path = parent->path + "/";
    path += name;

    static_scope = ip->get_static_scope( path );
    do_not_execute_anything = false;
}

Var Scope::VecNamedVar::add( int name, Var var ) {
    NamedVar *res = data.push_back();
    res->name = name;
    res->var = var;
    return res->var;
}

bool Scope::VecNamedVar::contains( int name ) {
    for( NamedVar &nv : data )
        if ( nv.name == name )
            return true;
    return false;
}

Var Scope::VecNamedVar::get( int name ) {
    for( NamedVar &nv : data )
        if ( nv.name == name )
            return nv.var;
    return Var();
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
    int name = read_nstring( bin );
    if ( Var res = find_var( name ) )
        return res;
    return ip->ret_error( "Impossible to find variable '" + ip->str_cor.str( name ) + "'." );
}
Var Scope::find_first_var( int name ) {
    for( Scope *s = this; ; s = s->parent ) {
        if ( Var res = s->local_scope.get( name ) )
            return res;
        if ( Var res = s->static_scope->get( name ) )
            return res;
    }
    return Var();
}

Var Scope::find_var( int name ) {
    Var res = find_first_var( name );
    if ( res.is_surdef() ) {
        // surdef_list = nb_surdefs, [ surdef_expr ]*n
        //Vec<Var> lst;
        //find_var_clist( lst, name );
        //return ip->make_Callable( lst, self );
        TODO;
    }
    return res;

}

int Scope::read_nstring( BinStreamReader &bin ) {
    return ip->sf->cor_str[ bin.read_positive_integer() ];
}

Var Scope::parse_ASSIGN( BinStreamReader bin ) {
    // name
    int name = read_nstring( bin );

    // flags
    int flags = bin.read_positive_integer();

    // inst
    Var var = parse( bin.read_offset() );

    //
    if ( flags & IR_ASSIGN_TYPE )
        TODO;
        // var = apply( var, 0, 0, 0, 0, 0, class_scope ? APPLY_MODE_PARTIAL_INST : APPLY_MODE_STD, sf, off );

    //if ( ( flags & IR_ASSIGN_REF ) == 0 and var.referenced_more_than_one_time() )
    //    var = copy( var, sf, off );

    if ( flags & IR_ASSIGN_CONST ) {
        TODO;
        //        if ( ( var.flags & Var::WEAK_CONST ) == 0 and var.referenced_more_than_one_time() )
        //            disp_error( "Impossible to make this variable fully const (already referenced elsewhere)", sf, off );
        //        var.flags |= Var::WEAK_CONST;
        //        var.data->flags = PRef::CONST;
    }

    return reg_var( name, var, flags & IR_ASSIGN_STATIC );
}
Var Scope::reg_var( int name, Var var, bool stat ) {
    if ( local_scope.contains( name ) or static_scope->contains( name ) )
        return ip->ret_error( "Scope already contains a var named '" + ip->str_cor.str( name ) + "'" );
    VecNamedVar &scope = stat ? *static_scope : local_scope;
    return scope.add( name, var );
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
    Var cond = parse( bin.read_offset() );
    if ( cond.type == &ip->type_Error )
        return cond;

    // bool conversion
    if ( cond.type != &ip->type_Bool ) {
        TODO;
        //        cond = apply( find_var( STRING_Bool_NUM ), 1, &cond, 0, 0, 0, APPLY_MODE_STD, sf, off );
        //        if ( ip->isa_Error( cond ) )
        //            return cond;
    }

    // simplified expression
    Expr expr = cond.get_val();
    PRINT( expr );

    //
    const PI8 *ok = bin.read_offset();
    const PI8 *ko = bin.read_offset();

    // known value
    bool cond_val;
    if ( expr->get_val( cond_val ) ) {
        if ( cond_val ) {
            Scope if_scope( this, "if_" + to_string( ok ) );
            return if_scope.parse( ok );
        }
        // else
        if ( ko ) {
            Scope if_scope( this, "fi_" + to_string( ko ) );
            return if_scope.parse( ko );
        }
        return ip->void_var();
    }

    Var res_ok;
    if ( ok ) {
        ip->set_cond( expr );
        Scope if_scope( this, "if_" + to_string( ok ) );
        res_ok = if_scope.parse( ok );
        ip->pop_cond();
    }

    Var res_ko;
    if ( ko ) {
        ip->set_cond( op( &ip->type_Bool, &ip->type_Bool, expr, Op_not_boolean() ) );
        Scope if_scope( this, "fi_" + to_string( ko ) );
        res_ko = if_scope.parse( ko );
        ip->pop_cond();
    }

    if ( res_ok.type != res_ko.type or not ko )
        return ip->void_var();
    return Var( Ref(), res_ok.type, select( expr, res_ok.ref(), res_ko.ref() ) );
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
    Bool val = false;
    return Var( &ip->type_Bool, cst( 1, (PI8 *)&val ) );
}
Var Scope::parse_TRUE( BinStreamReader bin ) {
    Bool val = true;
    return Var( &ip->type_Bool, cst( 1, (PI8 *)&val ) );
}
Var Scope::parse_VOID( BinStreamReader bin ) {
    return Var( &ip->type_Void );
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
    return Var( &ip->type_Bool, symbol( "rand", 1 ) );
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
    int n = bin.read_positive_integer();
    if ( n == 1 ) {
        TODO;
    }
    if ( n != 2 )
        return ip->ret_error( "expecting 1 or 2 args" );
    Var dst = parse( bin.read_offset() );
    Var src = parse( bin.read_offset() );
    PRINT( dst );
    PRINT( src );
    dst.set_val( src );
    return dst;
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

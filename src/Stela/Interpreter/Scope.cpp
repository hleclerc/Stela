#include "Interpreter.h"
#include "SourceFile.h"
#include "Scope.h"

#include "../Ir/CallableFlags.h"
#include "../Ir/AssignFlags.h"
#include "../Ir/Numbers.h"

Scope::Scope( Interpreter *ip, Scope *parent, Scope *caller ) : ip( ip ), parent( parent ), caller( caller ) {
    do_not_execute_anything = false;
    instantiated_from_sf = 0;
}

Var Scope::parse( const Var *sf, const PI8 *tok ) {
    if ( tok == 0 or do_not_execute_anything )
        return ip->error_var;

    BinStreamReader bin( tok );
    PI8 tva = bin.get<PI8>();
    int off = bin.read_positive_integer();
    PRINT( (int)tva );

    switch ( tva ) {
        #define DECL_IR_TOK( N ) case IR_TOK_##N: return parse_##N( sf, off, bin );
        #include "../Ir/Decl.h"
        #undef DECL_IR_TOK
        default: ip->disp_error( "Unknown token type", sf, off, this ); return ip->error_var;
    }
}

Var Scope::parse_BLOCK( const Var *sf, int off, BinStreamReader bin ) {
    Var res;
    while ( const PI8 *tok = bin.read_offset() )
        res = parse( sf, tok );
    return res;
}

Var Scope::parse_DEF( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_CLASS( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_RETURN( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_APPLY( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_SELECT( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_CHBEBA( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_SI32( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_PI32( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_SI64( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_PI64( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_PTR( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_STRING( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_VAR( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_ASSIGN( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_REASSIGN( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_GET_ATTR( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_GET_ATTR_PTR( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_GET_ATTR_ASK( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_GET_ATTR_PTR_ASK( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_GET_ATTR_PA( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_IF( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_WHILE( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_BREAK( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_CONTINUE( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_FALSE( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_TRUE( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_VOID( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_SELF( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_THIS( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_FOR( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_IMPORT( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_NEW( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_LIST( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_LAMBDA( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_NULL_REF( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }

#define DECL_IR_TOK( N ) Var Scope::parse_##N( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
#include "../Ir/Decl_Primitives.h"
#include "../Ir/Decl_UnaryOperations.h"
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK


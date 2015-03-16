#include "../System/Assert.h"
#include "RecIrParser.h"
#include "Numbers.h"

void RecIrParser::parse( const PI8 *ir ) {
    if ( ir ) {
        switch ( *ir ) {
            #define DECL_IR_TOK( N ) case IR_TOK_##N: parse_##N( ir + 1 ); break;
            #include "../Ir/Decl.h"
            #undef DECL_IR_TOK
        }
    }
}

void RecIrParser::_parse_CALLABLE( BinStreamReader bin, bool def ) {
    TODO;
//    os << ' ' << nstring( read_nstring( bin ) );
//    int flags = bin.read_positive_integer();
//    if ( flags & IR_ABSTRACT )
//        os << "[abstract]";
//    int nargs = bin.read_positive_integer();
//    int dargs = bin.read_positive_integer();
//    double pert = flags & IR_HAS_COMPUTED_PERT ? 0 : double( bin.read_positive_integer() ) / double( bin.read_positive_integer() );
//    os << "(";
//    if ( flags & IR_SELF_AS_ARG )
//        os << " self";
//    for( int i = 0; i < nargs; ++i )
//        os << ( i or ( flags & IR_SELF_AS_ARG ) ? ", " : " " ) << nstring( read_nstring( bin ) );
//    if ( flags & IR_VARARGS )
//        os << ( nargs or ( flags & IR_SELF_AS_ARG ) ? ", " : " " ) << "varargs";
//    os << " )";
//    if ( not ( flags & IR_HAS_COMPUTED_PERT ) )
//        os << " pertinence " << pert;
//    os << "\n";

//    for( int i = 0; i < dargs; ++i )
//        os << sp << "default_for_arg_" << nargs - dargs + i << " " << RecIrParser( cor, bin.read_offset(), rec, sp + "  " );

//    if ( flags & IR_HAS_COMPUTED_PERT )
//        os << sp << "pertinence " << RecIrParser( cor, bin.read_offset(), rec, sp + "  " );

//    if ( flags & IR_HAS_CONDITION )
//        os << sp << "when " << RecIrParser( cor, bin.read_offset(), rec, sp + "  " );

//    os << sp << "block " << RecIrParser( cor, bin.read_offset(), rec, sp + "  " );

//    if ( def ) {
//        os << sp << "block_with_ret " << RecIrParser( cor, bin.read_offset(), rec, sp + "  " );
//        if ( flags & IR_HAS_RETURN_TYPE )
//            os << sp << "ret_type " << RecIrParser( cor, bin.read_offset(), rec, sp + "  " );
//    } else {
//        int nb_anc = bin.read_positive_integer();
//        for( int i = 0; i < nb_anc; ++i )
//            os << sp << "ret_type " << RecIrParser( cor, bin.read_offset(), rec, sp + "  " );
//    }
}

void RecIrParser::_parse_APPLY( BinStreamReader bin, bool ret ) {
    // func
    parse( bin.read_offset() );

    int nu = bin.read_positive_integer();
    for( int i = 0; i < nu; ++i )
        parse( bin.read_offset() );

    int nn = bin.read_positive_integer();
    for( int i = 0; i < nn; ++i ) {
        int name = bin.read_positive_integer();
        parse( bin.read_offset() );
    }
}

void RecIrParser::parse_BLOCK( BinStreamReader bin ) {
    while ( const PI8 *tok = bin.read_offset() )
        parse( tok );
}
void RecIrParser::parse_DEF( BinStreamReader bin ) {
    _parse_CALLABLE( bin, true );
}
void RecIrParser::parse_CLASS( BinStreamReader bin ) {
    _parse_CALLABLE( bin, false );
}
void RecIrParser::parse_RETURN( BinStreamReader bin ) {
    parse( bin.read_offset() );
}
void RecIrParser::parse_APPLY( BinStreamReader bin ) {
    _parse_APPLY( bin, true );
}
void RecIrParser::parse_APPLY_WITH_RET( BinStreamReader bin ) {
    _parse_APPLY( bin, true );
}
void RecIrParser::parse_SELECT( BinStreamReader bin ) {
    _parse_APPLY( bin, true );
}
void RecIrParser::parse_SELECT_WITH_RET( BinStreamReader bin ) {
    _parse_APPLY( bin, true );
}
void RecIrParser::parse_CHBEBA( BinStreamReader bin ) {
    _parse_APPLY( bin, true );
}
void RecIrParser::parse_CHBEBA_WITH_RET( BinStreamReader bin ) {
    _parse_APPLY( bin, true );
}
void RecIrParser::parse_SI32( BinStreamReader bin ) {
}
void RecIrParser::parse_PI32( BinStreamReader bin ) {
}
void RecIrParser::parse_SI64( BinStreamReader bin ) {
}
void RecIrParser::parse_PI64( BinStreamReader bin ) {
}
void RecIrParser::parse_PTR( BinStreamReader bin ) {
}
void RecIrParser::parse_STRING( BinStreamReader bin ) {
}
void RecIrParser::parse_VAR( BinStreamReader bin ) {
}
void RecIrParser::parse_ASSIGN( BinStreamReader bin ) {
    // name
    int name = bin.read_positive_integer();

    // flags
    int flags = bin.read_positive_integer();
    //if ( flags & IR_ASSIGN_REF ) os << " [ref]";
    //if ( flags & IR_ASSIGN_STATIC ) os << " [static]";
    //if ( flags & IR_ASSIGN_CONST ) os << " [const]";
    //if ( flags & IR_ASSIGN_TYPE ) os << " [type]";

    // inst
    parse( bin.read_offset() );
}
void RecIrParser::parse_REASSIGN( BinStreamReader bin ) {
    parse( bin.read_offset() );
    parse( bin.read_offset() );
}
void RecIrParser::parse_REASSIGN_WITH_RET( BinStreamReader bin ) {
    return parse_REASSIGN( bin );
}
void RecIrParser::_parse_GET_ATTR( BinStreamReader bin ) {
    int name = bin.read_positive_integer();
    parse( bin.read_offset() );
}
void RecIrParser::parse_GET_ATTR( BinStreamReader bin ) { _parse_GET_ATTR( bin ); }
void RecIrParser::parse_GET_ATTR_WITH_RET( BinStreamReader bin ) { _parse_GET_ATTR( bin ); }
void RecIrParser::parse_GET_ATTR_PTR( BinStreamReader bin ) { _parse_GET_ATTR( bin ); }
void RecIrParser::parse_GET_ATTR_PTR_WITH_RET( BinStreamReader bin ) { _parse_GET_ATTR( bin ); }
void RecIrParser::parse_GET_ATTR_ASK( BinStreamReader bin ) { _parse_GET_ATTR( bin ); }
void RecIrParser::parse_GET_ATTR_ASK_WITH_VAR( BinStreamReader bin ) { _parse_GET_ATTR( bin ); }
void RecIrParser::parse_GET_ATTR_PTR_ASK( BinStreamReader bin ) { _parse_GET_ATTR( bin ); }
void RecIrParser::parse_GET_ATTR_PTR_ASK_WITH_VAR( BinStreamReader bin ) { _parse_GET_ATTR( bin ); }
void RecIrParser::parse_GET_ATTR_PA( BinStreamReader bin ) { _parse_GET_ATTR( bin ); }
void RecIrParser::parse_GET_ATTR_PA_WITH_RET( BinStreamReader bin ) { _parse_GET_ATTR( bin ); }
void RecIrParser::parse_IF( BinStreamReader bin ) {
    parse( bin.read_offset() );
    parse( bin.read_offset() );
    parse( bin.read_offset() );
}
void RecIrParser::parse_IF_WITH_RET( BinStreamReader bin ) {
    parse_IF( bin );
}
void RecIrParser::parse_WHILE( BinStreamReader bin ) {
    parse( bin.read_offset() ); // OK
    parse( bin.read_offset() ); // KO
}
void RecIrParser::parse_WHILE_WITH_RET( BinStreamReader bin ) {
    parse_WHILE( bin );
}
void RecIrParser::parse_BREAK( BinStreamReader bin ) {}
void RecIrParser::parse_CONTINUE( BinStreamReader bin ) {}
void RecIrParser::parse_FALSE( BinStreamReader bin ) {}
void RecIrParser::parse_TRUE( BinStreamReader bin ) {}
void RecIrParser::parse_VOID( BinStreamReader bin ) {}
void RecIrParser::parse_SELF( BinStreamReader bin ) {}
void RecIrParser::parse_THIS( BinStreamReader bin ) {}
void RecIrParser::parse_NOP( BinStreamReader bin ) {}
void RecIrParser::parse_FOR( BinStreamReader bin ) {
    ST nn = bin.read_positive_integer();
    for( int i = 0; i < nn; ++i ) {
        int n = bin.read_positive_integer();
        // ...
    }
    ST nc = bin.read_positive_integer();
    for( int i = 0; i < nc; ++i )
        parse( bin.read_offset() ); // lists
    parse( bin.read_offset() ); // block
}

void RecIrParser::parse_IMPORT( BinStreamReader bin ) {
}

void RecIrParser::parse_NEW( BinStreamReader bin ) {
    parse_APPLY( bin );
}

void RecIrParser::parse_LIST( BinStreamReader bin ) {
    TODO;
}

void RecIrParser::parse_LAMBDA( BinStreamReader bin ) {
    TODO;
}

void RecIrParser::parse_NULL_PTR( BinStreamReader bin ) {
}

void RecIrParser::parse_NULL_REF( BinStreamReader bin ) {
}

#define DECL_IR_TOK( N ) void RecIrParser::parse_##N( BinStreamReader bin ) {}
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( N ) void RecIrParser::parse_##N( BinStreamReader bin ) {}
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( N ) void RecIrParser::parse_##N( BinStreamReader bin ) {}
#include "../Ir/Decl_Primitives.h"
#undef DECL_IR_TOK


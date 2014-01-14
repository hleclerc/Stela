#include "../System/BinStreamReader.h"
#include "../System/Assert.h"
#include "CallableFlags.h"
#include "AssignFlags.h"
#include "IrDisplayer.h"
#include "Numbers.h"

IrDisplayer::IrDisplayer( const PI8 *cor, const PI8 *ir, bool rec, std::string sp ) : sp( sp ), cor( cor ), ir( ir ), rec( rec ) {
}

IrDisplayer::IrDisplayer( const PI8 *bin, bool rec, std::string sp ) : sp( sp ), rec( rec ) {
    cor = bin;

    // skip the correction table
    if ( bin ) {
        BinStreamReader bsr( bin );
        while ( true ) {
            ST off = bsr.read_positive_integer();
            if ( not off )
                break;
            bsr.ptr += off;
        }
        ir = bsr.ptr;
    } else
        ir = 0;
}

void IrDisplayer::write_to_stream( Stream &os ) const {
    if ( ir ) {
        BinStreamReader bin( ir + 1 );
        switch ( *ir ) {
            #define DECL_IR_TOK( N ) case IR_TOK_##N: os << #N; off = bin.read_positive_integer(); parse_##N( os, bin ); break;
            #include "../Ir/Decl.h"
            #undef DECL_IR_TOK
        }
    } else
        os << "NULL\n";
}

std::string IrDisplayer::nstring( int n ) const {
    BinStreamReader bsr( cor );
    for( int i = 0; ; ++i ) {
        ST off = bsr.read_positive_integer();
        ASSERT( off, "invalid nstring number" );
        if ( i == n )
            return std::string( bsr.ptr, bsr.ptr + off );
        bsr.ptr += off;
    }
}

int IrDisplayer::read_nstring( BinStreamReader &bin ) const {
    return bin.read_positive_integer();
}

void IrDisplayer::_parse_CALLABLE( Stream &os, BinStreamReader bin, bool def ) const {
    os << ' ' << nstring( read_nstring( bin ) );
    int flags = bin.read_positive_integer();
    if ( flags & IR_ABSTRACT )
        os << "[abstract]";
    int nargs = bin.read_positive_integer();
    int dargs = bin.read_positive_integer();
    double pert = flags & IR_HAS_COMPUTED_PERT ? 0 : double( bin.read_positive_integer() ) / double( bin.read_positive_integer() );
    os << "(";
    if ( flags & IR_SELF_AS_ARG )
        os << " self";
    for( int i = 0; i < nargs; ++i )
        os << ( i or ( flags & IR_SELF_AS_ARG ) ? ", " : " " ) << nstring( read_nstring( bin ) );
    if ( flags & IR_VARARGS )
        os << ( nargs or ( flags & IR_SELF_AS_ARG ) ? ", " : " " ) << "varargs";
    os << " )";
    if ( not ( flags & IR_HAS_COMPUTED_PERT ) )
        os << " pertinence " << pert;
    os << "\n";

    for( int i = 0; i < dargs; ++i )
        os << sp << "default_for_arg_" << nargs - dargs + i << " " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );

    if ( flags & IR_HAS_COMPUTED_PERT )
        os << sp << "pertinence " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );

    if ( flags & IR_HAS_CONDITION )
        os << sp << "when " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );

    os << sp << "block " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );

    if ( def ) {
        if ( flags & IR_HAS_RETURN_TYPE )
            os << sp << "ret_type " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );
    } else {
        int nb_anc = bin.read_positive_integer();
        for( int i = 0; i < nb_anc; ++i )
            os << sp << "ret_type " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );
    }
}

void IrDisplayer::_parse_APPLY( Stream &os, BinStreamReader bin, const char *type ) const {
    os << " " << type << "\n";
    os << sp << "func " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " ); // function
    int nu = bin.read_positive_integer();
    for( int i = 0; i < nu; ++i )
        os << sp << "in_" << i << " " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " ); // unn arg

    int nn = bin.read_positive_integer();
    for( int i = 0; i < nn; ++i ) {
        os << sp << nstring( read_nstring( bin ) ) << " = ";
        os << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " ); // unn arg
    }
}

void IrDisplayer::parse_BLOCK( Stream &os, BinStreamReader bin ) const {
    os << "\n";
    if ( rec )
        while ( const PI8 *tok = bin.read_offset() )
            os << sp << IrDisplayer( cor, tok, rec, sp + "  " );
}
void IrDisplayer::parse_DEF( Stream &os, BinStreamReader bin ) const {
    _parse_CALLABLE( os, bin, true );
}
void IrDisplayer::parse_CLASS( Stream &os, BinStreamReader bin ) const {
    _parse_CALLABLE( os, bin, false );
}
void IrDisplayer::parse_RETURN( Stream &os, BinStreamReader bin ) const {
    os << " " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );
}
void IrDisplayer::parse_APPLY( Stream &os, BinStreamReader bin ) const {
    _parse_APPLY( os, bin, "apply" );
}
void IrDisplayer::parse_SELECT( Stream &os, BinStreamReader bin ) const {
    _parse_APPLY( os, bin, "select" );
}
void IrDisplayer::parse_CHBEBA( Stream &os, BinStreamReader bin ) const {
    _parse_APPLY( os, bin, "chbeba" );
}
void IrDisplayer::parse_SI32( Stream &os, BinStreamReader bin ) const {
    os << " " << SI32( bin.read_positive_integer() ) << "\n";
}
void IrDisplayer::parse_PI32( Stream &os, BinStreamReader bin ) const {
    os << " " << PI32( bin.read_positive_integer() ) << "\n";
}
void IrDisplayer::parse_SI64( Stream &os, BinStreamReader bin ) const {
    os << " " << SI64( bin.read_positive_integer() ) << "\n";
}
void IrDisplayer::parse_PI64( Stream &os, BinStreamReader bin ) const {
    os << " " << PI64( bin.read_positive_integer() ) << "\n";
}
void IrDisplayer::parse_PTR( Stream &os, BinStreamReader bin ) const {
    os << "\n";
}
void IrDisplayer::parse_STRING( Stream &os, BinStreamReader bin ) const {
    os << " ";

    ST off = bin.read_positive_integer();
    os.write( (const char *)bin.ptr, off );
    bin.ptr += off;

    os << "\n";
}
void IrDisplayer::parse_VAR( Stream &os, BinStreamReader bin ) const {
    os << " " << nstring( read_nstring( bin ) ) << "\n";
}
void IrDisplayer::parse_ASSIGN( Stream &os, BinStreamReader bin ) const {
    // name
    os << " " << nstring( read_nstring( bin ) );

    // flags
    int flags = bin.read_positive_integer();
    if ( flags & IR_ASSIGN_REF ) os << " [ref]";
    if ( flags & IR_ASSIGN_STATIC ) os << " [static]";
    if ( flags & IR_ASSIGN_CONST ) os << " [const]";
    if ( flags & IR_ASSIGN_TYPE ) os << " [type]";

    // inst
    os << "\n";
    os << sp << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );
}
void IrDisplayer::parse_REASSIGN( Stream &os, BinStreamReader bin ) const {
    os << "\n";
    os << sp << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " ); // arg 0
    os << sp << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " ); // arg 1
}
void IrDisplayer::parse_GET_ATTR( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_GET_ATTR_PTR( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_GET_ATTR_ASK( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_GET_ATTR_PTR_ASK( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_GET_ATTR_PA( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_IF( Stream &os, BinStreamReader bin ) const {
    os << "\n";
    os << sp << "cond " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );
    os << sp << "ok = " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );
    os << sp << "ko = " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );
}
void IrDisplayer::parse_WHILE( Stream &os, BinStreamReader bin ) const {
    os << "\n";
    os << sp << "ok = " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );
    os << sp << "ko = " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );
}
void IrDisplayer::parse_BREAK( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_CONTINUE( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_FALSE( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_TRUE( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_VOID( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_SELF( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_THIS( Stream &os, BinStreamReader bin ) const { os << "\n"; }
void IrDisplayer::parse_FOR( Stream &os, BinStreamReader bin ) const {
    ST nn = bin.read_positive_integer();
    for( int i = 0; i < nn; ++i )
        os << " " << nstring( read_nstring( bin ) );
    os << " in";
    ST nc = bin.read_positive_integer();
    for( int i = 0; i < nc; ++i )
        os << " " << IrDisplayer( cor, bin.read_offset(), rec, sp + "  " );
    os << "\n";
}

void IrDisplayer::parse_IMPORT( Stream &os, BinStreamReader bin ) const {
    ST size = bin.read_positive_integer();
    char data[ size + 1 ];
    bin.read( data, size );
    data[ size ] = 0;
    os << " " << data << "\n";
}

void IrDisplayer::parse_NEW( Stream &os, BinStreamReader bin ) const {
    parse_APPLY( os, bin );
}

void IrDisplayer::parse_LIST( Stream &os, BinStreamReader bin ) const {
    ST nb_dim = bin.read_positive_integer();
    os << " nb_dim = " << nb_dim;
}

void IrDisplayer::parse_LAMBDA( Stream &os, BinStreamReader bin ) const {
    os << " lambda";
}

void IrDisplayer::parse_NULL_REF( Stream &os, BinStreamReader bin ) const {
    os << " null_ref";
}

#define DECL_IR_TOK( N ) void IrDisplayer::parse_##N( Stream &os, BinStreamReader bin ) const { os << "\n"; }
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( N ) void IrDisplayer::parse_##N( Stream &os, BinStreamReader bin ) const { os << "\n"; }
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( N ) void IrDisplayer::parse_##N( Stream &os, BinStreamReader bin ) const { os << "\n"; }
#include "../Ir/Decl_Primitives.h"
#undef DECL_IR_TOK


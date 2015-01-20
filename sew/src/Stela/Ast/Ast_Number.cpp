#include "../Ssa/ParsingContext.h"
#include "../Ssa/Room.h"
#include "../Ssa/Cst.h"
#include "../Ir/Numbers.h"
#include "Ast_Number.h"
#include "IrWriter.h"
#include <sstream>

Ast_Number::Ast_Number( const char *src, int off ) : Ast( src, off ) {
    l = false;
    p = false;
    b = false;
}

Ast_Number::Ast_Number( const char *src, int off, bool val ) : Ast_Number( src, off ) {
    str = val ? "1" : "0";
    b = true;
}

Ast_Number::Ast_Number( const char *src, int off, String str ) : Ast_Number( src, off ) {
    this->str = str;
}

void Ast_Number::write_to_stream( Stream &os, int nsp ) const {
    os << str;
    if ( l ) os << 'l';
    if ( p ) os << 'p';
}


void Ast_Number::_get_info( IrWriter *aw ) const {
    if ( b )
        return;
    aw->data << atoi( str.c_str() );
}

Expr Ast_Number::_parse_in( ParsingContext &context ) const {
    std::istringstream ss( str );
    if ( l ) {
        SI64 res;
        ss >> res;
        return room( cst( ip->type_SI64, 64, &res ) );
    }
    if ( p ) {
        ST res;
        ss >> res;
        return room( cst( ip->type_ST, 8 * sizeof( ST ), &res ) );
    }
    SI32 res;
    ss >> res;
    return room( cst( ip->type_SI32, 32, &res ) );
}

PI8 Ast_Number::_tok_number() const {
    if ( b )
        return str == "1" ? IR_TOK_TRUE : IR_TOK_FALSE;
    if ( p )
        return IR_TOK_PTR;
    if ( l )
        return IR_TOK_SI64;
    return IR_TOK_SI32;
}


#include "../Ssa/ParsingContext.h"
#include "../Ssa/Room.h"
#include "../Ssa/Cst.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Number.h"
#include "Ast_String.h"

Ast_String::Ast_String( int off, String str ) : Ast( off ), str( str ) {
}

void Ast_String::write_to_stream( Stream &os, int nsp ) const {
    os << str;
}


void Ast_String::_get_info( IrWriter *aw ) const {
    TODO;
}

Expr Ast_String::_parse_in( ParsingContext &context ) const {
    int size_cun = Ast_Number::size_for_CUInt( str.size() ), size_tot = size_cun + str.size();
    PI8 data[ size_tot ];
    Ast_Number::write_CUInt( data, str.size() );
    memcpy( data + size_cun, str.data(), str.size() );
    return room( cst( ip->type_CUInt, 8 * size_tot, data ) );
}

PI8 Ast_String::_tok_number() const {
    return IR_TOK_STRING;
}

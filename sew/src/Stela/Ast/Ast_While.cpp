#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_While.h"

Ast_While::Ast_While( int off ) : Ast( off ) {
}

void Ast_While::write_to_stream( Stream &os, int nsp ) const {
    os << "while";

    ok->write_to_stream( os, nsp + 2 );

    if ( ko ) {
        os << "\n" << String( nsp, ' ' ) << "else ";
        ko->write_to_stream( os, nsp + 2 );
    }
}

void Ast_While::_get_info( AstWriter *aw ) const {
}

PI8 Ast_While::_tok_number() const {
    return IR_TOK_WHILE;
}


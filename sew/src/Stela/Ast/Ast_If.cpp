#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_If.h"

Ast_If::Ast_If( int off ) : Ast( off ) {
}

void Ast_If::write_to_stream( Stream &os, int nsp ) const {
    os << "if ";

    cond->write_to_stream( os, nsp + 6 );

    ok->write_to_stream( os, nsp + 2 );

    if ( ko ) {
        os << "\n" << String( nsp, ' ' ) << "else ";
        ko->write_to_stream( os, nsp + 2 );
    }
}


void Ast_If::_get_info( AstWriter *aw ) const {
    TODO;
}

PI8 Ast_If::_tok_number() const {
    return IR_TOK_IF;
}

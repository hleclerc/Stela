#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_For.h"

Ast_For::Ast_For( int off ) : Ast( off ) {
}

void Ast_For::write_to_stream( Stream &os, int nsp ) const {
    os << "For";
}


void Ast_For::_get_info( AstWriter *aw ) const {
    TODO;
}

PI8 Ast_For::_tok_number() const {
    return IR_TOK_FOR;
}

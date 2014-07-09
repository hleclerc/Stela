#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_Void.h"

Ast_Void::Ast_Void( int off ) : Ast( off ) {
}

void Ast_Void::write_to_stream( Stream &os, int nsp ) const {
    os << "Void";
}

PI8 Ast_Void::_tok_number() const {
    return IR_TOK_VOID;
}

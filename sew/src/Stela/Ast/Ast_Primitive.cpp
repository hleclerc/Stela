#include "Ast_Primitive.h"

Ast_Primitive::Ast_Primitive( int off ) : Ast( off ) {
}

void Ast_Primitive::write_to_stream( Stream &os, int nsp ) const {
    os << "Primitive";
}

void Ast_Primitive::_get_info( AstWriter *aw ) const {
    TODO;
}

PI8 Ast_Primitive::_tok_number() const {
    return tok_number;
}


#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Import.h"

Ast_Import::Ast_Import( int off ) : Ast( off ) {
}

void Ast_Import::write_to_stream( Stream &os, int nsp ) const {
    os << "import";
    for( const String &f : files )
        os << " " << f;
}


void Ast_Import::_get_info( IrWriter *aw ) const {
    TODO;
}

PI8 Ast_Import::_tok_number() const {
    return IR_TOK_IMPORT;
}

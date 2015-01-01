#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Import.h"

Ast_Import::Ast_Import( const char *src, int off ) : Ast( src, off ) {
}

void Ast_Import::write_to_stream( Stream &os, int nsp ) const {
    os << "import";
    for( const String &f : files )
        os << " " << f;
}


void Ast_Import::_get_info( IrWriter *aw ) const {
    TODO;
}

Expr Ast_Import::_parse_in( ParsingContext &context ) const {
    return context.ret_error( "TODO: _parse_in", false, __FILE__, __LINE__ );
}

PI8 Ast_Import::_tok_number() const {
    return IR_TOK_IMPORT;
}

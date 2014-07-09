#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_Assign.h"

Ast_Assign::Ast_Assign( int off ) : Ast( off ) {
}

void Ast_Assign::write_to_stream( Stream &os, int nsp ) const {
    os << name;
    if ( stat )
        os << "static ";
    if ( cons )
        os << "const ";
    if ( type )
        os << " ~= ";
    else
        os << " := ";
    if ( ref )
        os << "ref ";
    val->write_to_stream( os, nsp );
}

void Ast_Assign::_get_info( AstWriter *aw ) const {
    TODO;
}

PI8 Ast_Assign::_tok_number() const {
    return IR_TOK_ASSIGN;
}

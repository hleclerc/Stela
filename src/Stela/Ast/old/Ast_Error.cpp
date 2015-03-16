#include "Ast_Error.h"

Ast_Error::Ast_Error() {
}

void Ast_Error::write_to_stream( Stream &os, int nsp ) const {
    os << "error";
}

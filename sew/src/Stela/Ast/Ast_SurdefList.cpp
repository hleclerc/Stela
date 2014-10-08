#include "Ast_SurdefList.h"

Ast_SurdefList::Ast_SurdefList( Vec<Past> lst ) : lst( lst ) {
}

void Ast_SurdefList::write_to_stream( Stream &os, int nsp ) const {
    os << "surdeflist " << lst;
}

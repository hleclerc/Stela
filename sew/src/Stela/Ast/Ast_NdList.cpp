#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_NdList.h"

Ast_NdList::Ast_NdList( int off ) : Ast( off ) {
}

void Ast_NdList::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    for( int i = 0; i < lst.size(); ++i )
        lst[ i ]->get_potentially_needed_ext_vars( res, avail );
}

void Ast_NdList::write_to_stream( Stream &os, int nsp ) const {
    os << "nb_dim=" << nb_dim << " sizes=" << sizes;
    for( int i = 0; i < lst.size(); ++i )
        lst[ i ]->write_to_stream( os << "\n" << String( nsp + 2, ' ' ), nsp + 2 );
}


void Ast_NdList::_get_info( IrWriter *aw ) const {
    TODO;
}

PI8 Ast_NdList::_tok_number() const {
    return IR_TOK_LIST;
}

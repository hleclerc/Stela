#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Block.h"

Ast_Block::Ast_Block( int off ) : Ast( off ) {
}

void Ast_Block::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    for( int i = 0; i < lst.size(); ++i )
        lst[ i ]->get_potentially_needed_ext_vars( res, avail );
}

void Ast_Block::prep_get_potentially_needed_ext_vars( std::set<String> &avail ) const {
    for( int i = 0; i < lst.size(); ++i )
        lst[ i ]->prep_get_potentially_needed_ext_vars( avail );
}

void Ast_Block::write_to_stream( Stream &os, int nsp ) const {
    for( int i = 0; i < lst.size(); ++i )
        lst[ i ]->write_to_stream( os << "\n" << String( nsp + 2, ' ' ), nsp + 2 );
}

void Ast_Block::_get_info( IrWriter *aw ) const {
    for( int i = 0; i < lst.size(); ++i )
        aw->push_delayed_parse( lst[ i ].ptr() );
    // end of the block (offset = 0)
    aw->data << 0;
}

Expr Ast_Block::_parse_in( ParsingContext &context ) const {
    Expr res;
    for( int i = 0; i < lst.size(); ++i )
        res = lst[ i ]->parse_in( context );
    return res; ///< return last var
}

PI8 Ast_Block::_tok_number() const {
    return IR_TOK_BLOCK;
}

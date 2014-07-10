#include "../Ir/Numbers.h"
#include "Ast_Primitive.h"
#include "IrWriter.h"

Ast_Primitive::Ast_Primitive( int off, int tok_number ) : Ast( off ), tok_number( tok_number ) {
}

void Ast_Primitive::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    for( int i = 0; i < args.size(); ++i )
        args[ i ]->get_potentially_needed_ext_vars( res, avail );
}

void Ast_Primitive::write_to_stream( Stream &os, int nsp ) const {
    os << "Primitive";
}

void Ast_Primitive::_get_info( IrWriter *aw ) const {
    aw->data << args.size();
    for( int i = 0; i < args.size(); ++i )
        aw->push_delayed_parse( args[ i ].ptr() );
}

PI8 Ast_Primitive::_tok_number() const {
    return tok_number;
}


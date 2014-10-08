#include "../Ir/Numbers.h"
#include "Ast_Primitive.h"
#include "IrWriter.h"

#include "../Conv/ConvContext.h"

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

Past Ast_Primitive::_parse_in( ConvScope &scope ) {
    if ( tok_number == IR_TOK_set_base_size_and_alig ) {
        if ( args.size() != 2 )
            return cc->ret_error( "___set_base_size_and_alig must take exactly 2 args" );
        scope.base_size = args[ 0 ]->parse_in( scope );
        scope.base_alig = args[ 1 ]->parse_in( scope );
        return cc->void_var();
    }
    TODO;
    return cc->error_var();
}


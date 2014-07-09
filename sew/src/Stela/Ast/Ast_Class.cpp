#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_Class.h"

Ast_Class::Ast_Class( int off ) : Ast_Callable( off ) {
}

void Ast_Class::_get_info( AstWriter *aw ) const {
    // extends
    aw->data << inheritance.size();
    for( int i = 0; i < inheritance.size(); ++i )
        aw->push_delayed_parse( inheritance[ i ].ptr() );
}

PI8 Ast_Class::_tok_number() const {
    return IR_TOK_CLASS;
}

int Ast_Class::_spec_flags() const {
    return 0;
}

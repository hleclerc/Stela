#include "../Ssa/ParsingContext.h"
#include "../Ssa/Class.h"
#include "../Ssa/Room.h"
#include "../Ssa/Cst.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Class.h"

Ast_Class::Ast_Class( const char *src, int off ) : Ast_Callable( src, off ) {
}

void Ast_Class::_get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    for( int i = 0; i < inheritance.size(); ++i )
        inheritance[ i ]->get_potentially_needed_ext_vars( res, avail );
}

void Ast_Class::_get_info( IrWriter *aw ) const {
    Ast_Callable::_get_info( aw );

    // extends
    aw->data << inheritance.size();
    for( int i = 0; i < inheritance.size(); ++i )
        aw->push_delayed_parse( inheritance[ i ].ptr() );
}

Expr Ast_Class::_parse_in( ParsingContext &context ) const {
    std::set<String> res;
    if ( context.parent ) {
        std::set<String> avail;
        avail.insert( name );
        get_potentially_needed_ext_vars( res, avail );
    }

    //
    Class *c = 0;
    #define DECL_BT( T ) if ( name == #T ) { c = ip->class_##T; c->ast_item = this; }
    #include "../Ssa/DeclBaseClass.h"
    #include "../Ssa/DeclParmClass.h"
    #undef DECL_BT
    if ( not c )
        c = new Class( this );

    //
    SI64 ptr = SI64( ST( c ) );
    Expr val = cst( ip->type_Class, 64, &ptr );
    Expr out = room( val );
    out->flags |= Inst::SURDEF;
    return context.reg_var( name, out, true );
}

PI8 Ast_Class::_tok_number() const {
    return IR_TOK_CLASS;
}

int Ast_Class::_spec_flags() const {
    return 0;
}

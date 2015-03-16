#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Select.h"

Ast_Select::Ast_Select( const char *src, int off ) : Ast_Call( src, off ) {
}

Expr Ast_Select::_parse_in( ParsingContext &context ) const {
    Expr f_expr = f->parse_in( context );
    Vec<Expr> arg_exprs( Rese(), args.size() );
    for( Past a : args )
        arg_exprs << a->parse_in( context );

    return context.apply( context.get_attr( f_expr, "select" ), args.size() - names.size(), arg_exprs.ptr(), names.size(), names.ptr(), arg_exprs.end() - names.size() );
}

PI8 Ast_Select::_tok_number() const {
    return IR_TOK_SELECT;
}

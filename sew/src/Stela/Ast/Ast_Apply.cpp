#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"

#include "Ast_Variable.h"
#include "Ast_Apply.h"

Ast_Apply::Ast_Apply( int off ) : Ast_Call( off ) {
}

Expr Ast_Apply::_parse_in( ParsingContext &context ) const {
    Expr f_expr = f->parse_in( context );
    Expr a_expr[ args.size() ];
    for( int i = 0; i < args.size(); ++i )
        a_expr[ i ] = args[ i ]->parse_in( context );

    int nu = args.size() - names.size();
    return context.apply( f_expr, nu, a_expr, names.size(), names.ptr(), a_expr + nu );
}

PI8 Ast_Apply::_tok_number() const {
    return IR_TOK_APPLY;
}


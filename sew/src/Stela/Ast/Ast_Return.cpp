#include "../Ssa/ParsingContext.h"
#include "../Ssa/Select.h"
#include "../Ssa/Cond.h"
#include "../Ssa/Op.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Return.h"

Ast_Return::Ast_Return( const char *src, int off ) : Ast( src, off ) {
}

void Ast_Return::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    val->get_potentially_needed_ext_vars( res, avail );
}

void Ast_Return::write_to_stream( Stream &os, int nsp ) const {
    os << "return ";
    val->write_to_stream( os, nsp + 2 );
}

Expr Ast_Return::ret( ParsingContext &context, Past past ) {
    Expr res = past->parse_in( context );
    Expr cond = context.cond;
    for( ParsingContext *s = &context; s; s = s->parent ) {
        if ( s->scope_type == ParsingContext::SCOPE_TYPE_DEF ) {
            if ( s->ret )
                s->ret = select( cond, res, s->ret );
            else
                s->ret = res;

            s->cond = and_boolean( s->cond, not_boolean( cond ) );
            break;
        }

        s->cond = and_boolean( s->cond, not_boolean( cond ) );
    }
    return res;
}


void Ast_Return::_get_info( IrWriter *aw ) const {
    aw->push_delayed_parse( val.ptr() );
}

Expr Ast_Return::_parse_in( ParsingContext &context ) const {
    return ret( context, val );
}

PI8 Ast_Return::_tok_number() const {
    return IR_TOK_RETURN;
}

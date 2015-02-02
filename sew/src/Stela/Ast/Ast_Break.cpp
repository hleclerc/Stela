#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Break.h"

Ast_Break::Ast_Break( const char *src, int off ) : Ast( src, off ) {
    n = 1;
}

void Ast_Break::write_to_stream( Stream &os, int nsp ) const {
    os << "Break[" << n << "]";
}

void Ast_Break::_get_info( IrWriter *aw ) const {
    aw->data << n;
}

Expr Ast_Break::_parse_in( ParsingContext &context ) const {
    // get the number of loop to break
    int i = n, r = 0; // r is the number of loop to break (measured by the number of defined `cont`)
    for( ParsingContext *s = &context; i and s; ) {
        // break from a for block
        if ( ParsingContext *f = s->for_block ) {
            --i;
            for( ; s != f; s = s->caller ? s->caller : s->parent ) {
                if ( not s )
                    return context.ret_error( "Impossible to find the surrounding for scope" );
                r += bool( s->cont );
            }
            s = f;
        } else {
            // break from a while
            if ( s->cont ) {
                --i; // broken loop
                ++r;
            }
            //
            s = s->caller ? s->caller : s->parent;
        }
    }

    //
    context.BREAK( r, context.cond );
    return ip->void_var();
}

PI8 Ast_Break::_tok_number() const {
    return IR_TOK_BREAK;
}

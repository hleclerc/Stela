#include "../Ast/Ast_SurdefList.h"
#include "../Ast/Ast_Number.h"
#include "ConvScope.h"

ConvScope::ConvScope( ConvScope *parent ) : parent( parent ) {
    base_size = new Ast_Number( -1, "0" );
    base_alig = new Ast_Number( -1, "1" );
}

bool ConvScope::reg_var( String name, Past expr, bool check_existing, bool stat ) {
    if ( check_existing )
        for( NamedVar &v : variables )
            if ( v.name == name )
                return false;
    variables << NamedVar{ name, expr, stat };
    return true;
}

Past ConvScope::find_var( String name ) {
    Vec<Past> res;
    for( ConvScope *s = this; s; s = s->parent ) {
        for( NamedVar &v : s->variables ) {
            if ( v.name == name ) {
                if ( res.size() == 0 and not v.expr->may_be_surdefined() )
                    return v.expr;
                res << v.expr;
            }
        }
    }
    return new Ast_SurdefList( res );
}

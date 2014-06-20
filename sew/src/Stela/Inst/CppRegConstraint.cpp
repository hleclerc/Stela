#include "CppRegConstraint.h"

void CppRegConstraint::add( SI32 level, Expr expr_0, SI32 ninp_0, Expr expr_1, SI32 ninp_1 ) {
    PlugWithLevel &p_0 = constraints[ Plug{ expr_0, ninp_0 } ];
    p_0.expr  = expr_1;
    p_0.ninp  = ninp_1;
    p_0.level = level;

    PlugWithLevel &p_1 = constraints[ Plug{ expr_1, ninp_1 } ];
    p_1.expr  = expr_0;
    p_1.ninp  = ninp_0;
    p_1.level = level;
}

void CppRegConstraint::write_to_stream( Stream &os ) const {
    for( auto c : constraints ) {
        os << "\n    ";

        os << c.first.expr;
        if ( c.first.ninp < 0 )
            os << " out";
        else
            os << "[" << c.first.ninp << "]";

        os << " -> ";

        os << c.second.expr;
        if ( c.second.ninp < 0 )
            os << " out";
        else
            os << "[" << c.second.ninp << "]";

        os << " (level=" << c.second.level << ")";
    }
}

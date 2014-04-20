#include "../Inst/Phi.h"
#include "RefPhi.h"

RefPhi::RefPhi( const Expr &cond, const Var &ok, const Var &ko ) : cond( cond ), ok( ok ), ko( ko ) {
}

void RefPhi::write_to_stream( Stream &os ) const {
    os << "select( " << cond << ", " << ok << ", " << ko << ")";
}

bool RefPhi::contains_var_referenced_more_than_one_time() const {
    return ok.referenced_more_than_one_time() or ko.referenced_more_than_one_time();
}

Expr RefPhi::expr() const {
    return phi( cond, ok.expr(), ko.expr() );
}

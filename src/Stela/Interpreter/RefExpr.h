#ifndef REFEXPR_H
#define REFEXPR_H

#include "Expr.h"
#include "Ref.h"

/**
*/
class RefExpr : public Ref {
public:
    RefExpr( const Expr &expr );
    virtual Expr get() const;

    Expr expr;
};

#endif // REFEXPR_H

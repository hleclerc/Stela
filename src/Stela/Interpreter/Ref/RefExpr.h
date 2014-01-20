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
    virtual void set( Expr expr );
    virtual void write_to_stream( Stream &os ) const;

    Expr expr;
};

#endif // REFEXPR_H

#ifndef REFEXPR_H
#define REFEXPR_H

#include "Ref.h"

/**
*/
class RefExpr : public Ref {
public:
    RefExpr( const Expr &data );
    virtual Expr expr() const;
    virtual void write_to_stream( Stream &os ) const;
    virtual bool contains_var_referenced_more_than_one_time() const;

    Expr data;
};

#endif // REFEXPR_H

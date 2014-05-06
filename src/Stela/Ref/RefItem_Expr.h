#ifndef REFITEM_EXPR_H
#define REFITEM_EXPR_H

#include "../Inst/Expr.h"
#include "RefItem.h"

/**
*/
class RefItem_Expr : public RefItem {
public:
    RefItem_Expr( const Expr &expr );
    virtual void write_to_stream( Stream &os ) const;
    virtual RefItem *shallow_copy() const;
    virtual Expr expr() const;

    // attributes
    Expr _expr;
};

#endif // REFITEM_EXPR_H

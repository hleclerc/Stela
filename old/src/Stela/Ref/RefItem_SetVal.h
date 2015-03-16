#ifndef REFITEM_SETVAL_H
#define REFITEM_SETVAL_H

#include "RefItem.h"

/**
*/
class RefItem_SetVal : public RefItem {
public:
    RefItem_SetVal( RefItem *old, RefItem *src, int off );
    virtual ~RefItem_SetVal();

    virtual void write_to_stream( Stream &os ) const;
    virtual RefItem *shallow_copy() const;
    virtual bool contains_a_ptr() const;
    virtual Var pointed_val() const;
    virtual Expr expr() const;

    RefItem *old;
    RefItem *src;
    int off;
};

#endif // REFITEM_SETVAL_H

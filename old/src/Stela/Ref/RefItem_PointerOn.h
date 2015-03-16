#ifndef REFITEM_POINTERON_H
#define REFITEM_POINTERON_H

#include "Var.h"

/**
*/
class RefItem_PointerOn : public RefItem {
public:
    RefItem_PointerOn( const Var &ref );
    virtual void write_to_stream( Stream &os ) const;
    virtual RefItem *shallow_copy() const;
    virtual bool contains_a_ptr() const;
    virtual Var pointed_val() const;
    virtual Expr expr() const;

    Var var;
};

#endif // REFITEM_POINTERON_H

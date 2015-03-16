#ifndef REFITEM_SLICE_H
#define REFITEM_SLICE_H

#include "Var.h"

/**
*/
class RefItem_Slice : public RefItem {
public:
    RefItem_Slice( const Var &var, int beg, int end );

    virtual void write_to_stream( Stream &os ) const;
    virtual RefItem *shallow_copy() const;
    virtual bool contains_a_ptr() const;
    virtual Var pointed_val() const;
    virtual Expr expr() const;

    virtual void reassign( Ref &dst, RefItem *src );

    Var var;
    int beg;
    int end;
};

#endif // REFITEM_SLICE_H

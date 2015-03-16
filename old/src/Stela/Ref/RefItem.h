#ifndef REFITEM_H
#define REFITEM_H

#include "../System/Stream.h"
class Expr;
class Ref;
class Var;

/**

*/
class RefItem {
public:
    RefItem();
    virtual ~RefItem();

    virtual void write_to_stream( Stream &os ) const;
    virtual RefItem *shallow_copy() const = 0; ///<
    virtual bool contains_a_ptr() const;
    virtual Var pointed_val() const;
    virtual Expr expr() const = 0; ///< deep snapshot

    virtual void reassign( Ref &dst, RefItem *src );
};

#endif // REFITEM_H

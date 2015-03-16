#ifndef RefPointerOn_H
#define RefPointerOn_H

#include "Ref.h"
#include "Var.h"

/**
*/
class RefPointerOn : public Ref {
public:
    RefPointerOn( Var var );

    virtual Expr expr() const;
    virtual void write_to_stream( Stream &os ) const;
    virtual bool contains_var_referenced_more_than_one_time() const;
    virtual Var pointed_value() const;

    Var var;
};

Var pointer_on( Var var );

#endif // RefPointerOn_H

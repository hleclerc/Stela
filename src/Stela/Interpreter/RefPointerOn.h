#ifndef RefPointerOn_H
#define RefPointerOn_H

#include "Ref.h"
#include "Var.h"

/**
*/
class RefPointerOn : public Ref {
public:
    RefPointerOn( Var var, int ptr_size );

    virtual Expr expr() const;
    virtual void write_to_stream( Stream &os ) const;
    virtual bool indirect_set( Expr expr, Scope *set_scope, const Expr &sf, int off, Expr ext_cond );

    Var var;
    int ptr_size;
};

Var pointer_on( Var var );

#endif // RefPointerOn_H

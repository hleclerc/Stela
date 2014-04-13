#ifndef REF_H
#define REF_H

#include "../Inst/Expr.h"
class Scope;

/**
*/
class Ref : public ObjectWithCptUse {
public:
    Ref();
    virtual ~Ref();

    virtual Expr expr() const = 0;
    virtual void write_to_stream( Stream &os ) const = 0;
    virtual bool indirect_set( Expr expr, Scope *set_scope, const Expr &sf, int off, Expr ext_cond );
    virtual void direct_set( Expr expr );
};

#endif // REF_H

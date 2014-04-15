#ifndef REF_H
#define REF_H

#include "../Inst/Expr.h"
class Scope;
class Var;

/**
*/
class Ref : public ObjectWithCptUse {
public:
    Ref();
    virtual ~Ref();

    virtual Expr expr() const = 0;
    virtual void write_to_stream( Stream &os ) const = 0;
    virtual bool contains_var_referenced_more_than_one_time() const = 0;
    virtual bool indirect_set( const Var &src, Scope *set_scope, const Expr &sf, int off, Expr ext_cond );
    virtual Var pointed_value() const;
};

#endif // REF_H

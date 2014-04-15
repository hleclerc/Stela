#ifndef REFPHI_H
#define REFPHI_H

#include "Ref.h"
#include "Var.h"

/**
*/
class RefPhi : public Ref {
public:
    RefPhi( const Expr &cond, Ptr<Ref> ok, Ptr<Ref> ko );
    virtual void write_to_stream( Stream &os ) const;
    virtual bool contains_var_referenced_more_than_one_time() const;
    virtual bool indirect_set( const Var &src, Scope *set_scope, const Expr &sf, int off, Expr ext_cond );
    virtual Expr expr() const;

    Expr     cond;
    Ptr<Ref> ok;
    Ptr<Ref> ko;
};

#endif // REFPHI_H

#ifndef REFPHI_H
#define REFPHI_H

#include "Ref.h"
#include "Var.h"

/**
*/
class RefPhi : public Ref {
public:
    RefPhi( const Expr &cond, const Var &ok, const Var &ko );
    virtual void write_to_stream( Stream &os ) const;
    virtual bool contains_var_referenced_more_than_one_time() const;
    virtual Expr expr() const;

    Expr cond;
    Var  ok;
    Var  ko;
};

#endif // REFPHI_H

#ifndef REFSLICEUNK_H
#define REFSLICEUNK_H


#include "Ref.h"
#include "Var.h"

/**
*/
class RefSliceUnk : public Ref {
public:
    RefSliceUnk( const Var &var, const Expr &beg, int len );
    virtual Expr expr() const;
    virtual void write_to_stream( Stream &os ) const;
    virtual bool contains_var_referenced_more_than_one_time() const;
    virtual bool indirect_set( const Var &src, Scope *set_scope, const Expr &sf, int off, Expr ext_cond );
    virtual Ptr<Ref> copy();

    Var  var;
    Expr beg;
    int  len;
};

#endif // REFSLICEUNK_H

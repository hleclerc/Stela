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
    virtual bool indirect_set( Expr expr, Scope *set_scope, const Expr &sf, int off, Expr ext_cond );

    Var  var;
    Expr beg;
    int  len;
};

#endif // REFSLICEUNK_H

#ifndef REFSLICE_H
#define REFSLICE_H

#include "Ref.h"
#include "Var.h"

/**
*/
class RefSlice : public Ref {
public:
    RefSlice( const Var &var, int beg, int end );
    virtual Expr expr() const;
    virtual void set( Expr expr );
    virtual void write_to_stream( Stream &os ) const;

    Var var;
    int beg;
    int end;
};

#endif // REFSLICE_H

#ifndef REFPTR_H
#define REFPTR_H

#include "Ref.h"
#include "Var.h"

/**
*/
class RefPtr : public Ref {
public:
    RefPtr( Var var, int ptr_size );

    virtual Expr expr() const;
    virtual void set( Expr expr );
    virtual void write_to_stream( Stream &os ) const;

    Var var;
    int ptr_size;
};

Var ptr( Interpreter *ip, Var var );

#endif // REFPTR_H

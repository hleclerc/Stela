#ifndef CLASS_H
#define CLASS_H

#include "Callable.h"
class Type;

/**
*/
class Class : public Callable {
public:
    struct TrialClass : Trial {
        TrialClass( Class *orig ) : orig( orig ) {}
        virtual Expr call( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, int apply_mode );
        Vec<Expr> args;
        Class *orig;
    };

    Class();
    void write_to_stream( Stream &os );
    virtual void read_bin( Scope *scope, BinStreamReader &bin );
    virtual Trial *test( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn );

    Type *type_for( Vec<Expr> &args );

    static Expr const_or_copy( Expr &Expr );

    Vec<Type *> types;
    Vec<Code> ancestors;
};

#endif // CLASS_H

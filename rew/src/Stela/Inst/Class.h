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
        virtual Var call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self, int apply_mode );
        Vec<Var> args;
        Class *orig;
    };

    Class();
    virtual void read_bin( BinStreamReader &bin );
    virtual Trial *test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self );

    Type *type_for( const Vec<Var> &args );

    Vec<Type *> types;
    Vec<Code> ancestors;
};

#endif // CLASS_H

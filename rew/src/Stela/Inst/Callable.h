#ifndef CALLABLE_H
#define CALLABLE_H

#include "../System/BinStreamReader.h"
#include "Var.h"
class SourceFile;
class Scope;

/**
*/
class Callable {
public:
    struct ArgCst {
        Vec<int> class_names;
    };
    struct Code {
        Code( SourceFile *sf, const PI8 *tok ) : sf( sf ), tok( tok ) {}
        Code() : tok( 0 ) {}
        operator bool() const { return tok; }
        SourceFile *sf;
        const PI8 *tok;
    };
    struct Trial {
        Trial( const char *reason = 0 );
        virtual ~Trial();

        virtual Var call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self, int apply_mode );
        Trial *wr( const char *r ) { reason = r; return this; }
        bool ok() const { return not reason; }

        Var           cond;
        const char   *reason;
    };


    Callable();

    virtual void read_bin( BinStreamReader &bin );
    virtual Trial *test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self ) = 0;

    int min_nb_args() const;
    int max_nb_args() const;
    int nb_arg_vals() const;
    bool has_varargs() const;
    bool self_as_arg() const;

    // provenance
    SourceFile *sf;
    int off; ///< in sourcefile

    // data
    int         name;
    double      pertinence;

    int         flags;
    int         nargs;
    int         dargs;
    Vec<int >   arg_names;
    Vec<Code>   arg_defaults;
    Vec<ArgCst> arg_constraints; ///< class names
    Code        comp_pert;
    Code        condition;
    Code        block;
};

#endif // CALLABLE_H

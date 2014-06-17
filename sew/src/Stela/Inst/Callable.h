#ifndef CALLABLE_H
#define CALLABLE_H

#include "../System/BinStreamReader.h"
#include "BoolOpSeq.h"
#include "Inst.h"
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
        Code() : sf( 0 ), tok( 0 ) {}
        operator bool() const { return tok; }
        SourceFile *sf;
        const PI8 *tok;
    };
    struct CatchedVar {
        int type; ///< IN_...
        int np;   ///< num parent (only for type IN_LOCAL or IN_STATIC)
        int ns;   ///< num in scope
    };
    struct Trial {
        Trial( const char *reason = 0 );
        virtual ~Trial();

        virtual Expr call( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, int apply_mode, Scope *caller, const BoolOpSeq &cond, Expr catched_vars );
        Trial *wr( const char *r ) { reason = r; return this; }
        bool ok() const { return not reason; }

        BoolOpSeq   cond;
        const char *reason;
    };


    Callable();
    virtual ~Callable();

    virtual void read_bin( Scope *scope, BinStreamReader &bin );
    virtual Trial *test( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, Scope *caller ) = 0;
    virtual void write_to_stream( Stream &os );

    int min_nb_args() const;
    int max_nb_args() const;
    int nb_arg_vals() const;
    bool has_varargs() const;
    bool self_as_arg() const;

    // provenance
    SourceFile     *sf;
    int             off; ///< in sourcefile

    // data
    int             name;
    double          pertinence;

    int             flags;
    int             nargs;
    int             dargs;
    Vec<int>        arg_names;
    Vec<Code>       arg_defaults;
    Vec<ArgCst>     arg_constraints; ///< class names
    Code            comp_pert;
    Code            condition;
    Vec<CatchedVar> catched_vars;

    Inst           *var;
};

#endif // CALLABLE_H

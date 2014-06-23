#ifndef SCOPE_H
#define SCOPE_H

#include "../System/BinStreamReader.h"
#include "NamedVarList.h"
#include "BoolOpSeq.h"
class Callable;
class Class;
class Ip;

/**
*/
class Scope {
public:
    /// parent = where to find the local var
    /// closure ??
    Scope( Scope *parent, Scope *caller, String name, Ip *ip = 0 );

    Expr import( String file );
    Expr parse( SourceFile *sf, const PI8 *tok, const char *reason ); ///< version that change sf in ip
    Expr parse( const PI8 *tok ); ///< parse from the same sf
    int  read_nstring( BinStreamReader &bin );

    Scope        *parent;
    Scope        *caller;
    String        path;

    Vec<Expr>    *static_vars;
    Vec<Expr>     local_vars;

    SourceFile   *sf;     ///< reading context
    SI32          off;    ///< reading context
    const char   *reason; ///< reading context

    Expr          self; ///< pointer on Expr, to point out object to be worked on
    BoolOpSeq     cont; ///< continue or not variable
    BoolOpSeq     cond; ///< condition to execute instructions

    Callable     *callable;
    Type         *class_scope;
    bool          do_not_execute_anything;
    bool          method;

    bool          disp_tok;
    int           creation_date;

    enum ApplyMode { APPLY_MODE_STD, APPLY_MODE_PARTIAL_INST, APPLY_MODE_NEW };
    Expr apply( Expr f, int nu = 0, Expr *u_args = 0, int nn = 0, int *n_name = 0, Expr *n_args = 0, ApplyMode am = APPLY_MODE_STD );
    Expr get_attr( Expr self, int name );
    Expr find_var( int name );
    Expr copy( Expr &Expr );

protected:
    Expr parse_CALLABLE( BinStreamReader bin, Class *base_class );
    void find_var_clist( Vec<Expr> &lst, int name );
    Expr find_first_var( int name );
    void get_attr_rec( Vec<Expr> &res, Expr self, int name );
    Expr get_attr_rec( Expr self, int name );
    Expr _parse_VAR_IN__SCOPE( BinStreamReader &bin, bool stat );
    Expr _parse_VAR_IN_CATCHED_VARS( BinStreamReader &bin );
    Expr get_catched_var_in__scope( int np, int ns, bool stat );
    Expr get_catched_var_in_catched_vars( int s );


    template<class OP> Expr parse_una( BinStreamReader bin, OP o );
    template<class OP> Expr parse_bin( BinStreamReader bin, OP o );

    #define DECL_IR_TOK( N ) Expr parse_##N( BinStreamReader bin );
    #include "../Ir/Decl.h"
    #undef DECL_IR_TOK
};

#endif // SCOPE_H

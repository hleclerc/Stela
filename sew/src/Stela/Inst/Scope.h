#ifndef SCOPE_H
#define SCOPE_H

#include "../System/BinStreamReader.h"
#include "NamedVarList.h"
#include "BoolOpSeq.h"
#include "Callable.h"
class Class;
class Ip;

/**
*/
class Scope {
public:
    /// parent = where to find the named variables
    Scope( Scope *parent, Scope *caller, String name, Ip *ip = 0 );

    Expr import( String file );
    int  read_nstring( BinStreamReader &bin );
    Expr parse( SourceFile *sf, const PI8 *tok, const char *reason ); ///< version that change sf in ip

    Scope        *parent;
    Scope        *caller;
    String        path;

    NamedVarList *static_vars;
    NamedVarList  local_vars;
    NamedVarList *catched_vars;

    SourceFile   *sf;     ///< reading context
    SI32          off;    ///< reading context
    const char   *reason; ///< reading context

    Expr          self; ///< pointer on Expr, to point out object to be worked on
    BoolOpSeq    *cont; ///< continue or not variable
    BoolOpSeq     cond; ///< condition to execute instructions
    Scope        *for_block;

    Type         *class_scope;
    bool          do_not_execute_anything;
    bool          method;

    bool          disp_tok;
    int           creation_date;

    int           base_size;
    int           base_alig;

    enum ApplyMode { APPLY_MODE_STD, APPLY_MODE_PARTIAL_INST, APPLY_MODE_NEW };
    Expr apply( Expr f, int nu = 0, Expr *u_args = 0, int nn = 0, int *n_name = 0, Expr *n_args = 0, ApplyMode am = APPLY_MODE_STD );
    Expr get_attr( Expr self, int name );
    Expr find_var( int name, bool exclude_main_scope = false );
    Expr reg_var( int name, Expr var, bool stat = false );
    Expr copy( Expr &Expr );

protected:
    Expr _parse( const PI8 *tok ); ///< parse from the same sf
    Expr parse_CALLABLE( BinStreamReader bin, Type *base_type );
    void find_var_clist( Vec<Expr> &lst, int name );
    Expr find_first_var( int name, bool exclude_main_scope );
    void get_attr_rec( Vec<Expr> &res, Expr self, int name );
    Expr get_attr_rec( Expr self, int name );
    Expr get_first_attr( Expr self, int name );
    void get_attr_clist( Vec<Expr> &res, Expr self, int name );
    void BREAK( int n, BoolOpSeq cond );

    struct RemBreak {
        int       count;
        BoolOpSeq cond;
    };
    Vec<RemBreak> rem_breaks;

    template<class OP> Expr parse_una( BinStreamReader bin, OP o );
    template<class OP> Expr parse_bin( BinStreamReader bin, OP o );

    #define DECL_IR_TOK( N ) Expr parse_##N( BinStreamReader bin );
    #include "../Ir/Decl.h"
    #undef DECL_IR_TOK
};

#endif // SCOPE_H

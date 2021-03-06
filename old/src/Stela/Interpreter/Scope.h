#ifndef SCOPE_H
#define SCOPE_H

#include "../System/BinStreamReader.h"
#include "../System/ErrorList.h"
#include "VarTable.h"
#include <map>

/**
*/
class Scope {
public:
    Scope( Scope *parent, Scope *caller = 0 );

    Var parse( const Expr &sf, const PI8 *tok );

protected:
    enum ApplyMode { APPLY_MODE_STD, APPLY_MODE_PARTIAL_INST, APPLY_MODE_NEW };
    typedef std::map<Ptr<PRef>,Ptr<Ref> > TSvMap;
    friend class Interpreter;
    friend class ClassInfo;
    friend class TypeInfo;
    friend class DefInfo;
    friend class RefSliceUnk;
    friend class RefSlice;
    friend class RefPointerOn;

    struct RemBreak {
        int count;
        Expr cond;
    };

    #define DECL_IR_TOK( N ) Var parse_##N( const Expr &sf, int off, BinStreamReader bin ); ///< parse a given IR_TOK
    #include "../Ir/Decl.h"
    #undef DECL_IR_TOK

    template<class Op,int boolean> Var parse_una_op( const Expr &sf, int off, BinStreamReader bin, Op op, N<boolean> );
    template<class Op,int boolean> Var parse_bin_op( const Expr &sf, int off, BinStreamReader bin, Op op, N<boolean> );

    Var parse_CALLABLE( const Expr &sf, int off, BinStreamReader bin, Var *type, bool def );

    template<class T> Var make_var( T val );
    int read_nstring( const Expr &sf, BinStreamReader &bin );
    Var copy( const Var &var, const Expr &sf, int off );
    Var get_val_if_GetSetSopInst( const Var &val, const Expr &sf, int off );
    Expr simplified_expr( const Var &var, const Expr &sf, int off );
    Expr simplified_expr( const PRef &var, const Expr &sf, int off );
    Expr simplified_expr( const Expr &var, const Expr &sf, int off );
    Ptr<Ref> simplified_pref( const Var  &var, const Expr &sf, int off );
    Var apply( Var f, int nu, Var *u_args, int nn, int *n_names, Var *n_args, ApplyMode am, const Expr &sf, int off );
    Var get_attr_rec( Var self, int name );
    void get_attr_rec( Vec<Var> &res, Var self, int name );

    ErrorList::Error &make_error( String msg, const Expr &sf = Expr(), int off = 0, bool warn = false );
    Var disp_error( String msg, const Expr &sf = Expr(), int off = 0, bool warn = false );

    void set( Var &dst, const Var &src, const Expr &sf = Expr(), int off = 0, Expr ext_cond = Expr() );

    Var reg_var( int name, const Var &var, const Expr &sf, int off, bool stat = false, bool check = true );
    Var find_var( int name );
    Var find_var_first( int name ); ///< helper for find_var
    void find_var_clist( Vec<Var> &res, int name ); ///< helper for find_var
    Var get_attr( Var self, int attr, const Expr &sf, int off );
    Expr _phi_simplified_expr( const Expr &expr, const Expr &sf, int off );
    void BREAK( int n, const Expr &sf, int off, Expr ext_cond = Expr() );
    void _save_var_rec( Ptr<PRef> dst );

    Scope         *parent; ///< "accessible" scope, i.e. that can be read to find variables
    Scope         *caller; ///< caller scope, if this serves as the body of a function
    Ptr<VarTable>  static_named_vars;
    VarTable       named_vars;

    bool           do_not_execute_anything;
    Expr           instantiated_from_sf;
    int            instantiated_from_off;

    int            base_size;
    int            base_alig;

    Var            sys_state;
    Var            self;
    TypeInfo      *class_scope;
    Expr           cond;

    Var            cont; ///< continue variable
    Vec<RemBreak>  rem_breaks;
    Scope         *for_def_scope;
    Scope         *for_surrounding_scope; // for_def_scope->for_surrounding_scope points to scope `orig` created during the parse_for
    bool           for_block;

    TSvMap        *sv_map;
    PI64           sv_date; ///< PRef creation date must be < sv_date to be saved in sv_map
};

#endif // SCOPE_H

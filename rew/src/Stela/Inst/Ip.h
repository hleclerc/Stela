#ifndef IP_H
#define IP_H

#include "../System/ErrorList.h"
#include "../System/NstrCor.h"
#include "../System/AutoPtr.h"
#include "Sourcefile.h"
#include "Scope.h"
#include "Class.h"
#include "Type.h"
#include "Var.h"
#include <map>

/**
*/
class Ip {
public:
    typedef SplittedVec<Vec<Expr>,16> OldCondSet;

    Ip();

    Var ret_error( String msg, bool warn = false, const char *file = 0, int line = 0 );
    void disp_error( String msg, bool warn = false, const char *file = 0, int line = 0 );
    ErrorList::Error &error_msg( String msg, bool warn = false, const char *file = 0, int line = 0 );

    void set_cond( Expr cond );
    void set_cond( Var cond );
    Expr cur_cond();
    void pop_cond();

    Var error_var();
    Var void_var();

    Type *artificial_type_for_size( int size );

    Scope::VecNamedVar *get_static_scope( String path );

    void add_inc_path( String inc_path );
    void import( String file );

    SourceFile *new_sf( String file );
    int read_nstring( BinStreamReader &bin );

    Var make_Callable( Vec<Var> lst, Var self );
    Var make_Varargs( Vec<Var> lst, const Vec<int> &names = Vec<int>() );
    Type *make_Varargs_type( const Vec<Type *> &lst, const Vec<int> &names, int o );
    Var make_type_var( Type *type );
    Type *type_from_type_var( Var var );

    // base type
    #define DECL_BT( T ) \
        Type  type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    Type *type_ST;

    // base classes
    #define DECL_BT( T ) \
        Class class_##T;
    #include "DeclBaseClass.h"
    #include "DeclParmClass.h"
    #undef DECL_BT


    std::map<int,Type> art_types;
    std::map<String,SourceFile> sourcefiles;
    std::map<String,Scope::VecNamedVar> static_scopes;

    // std variables
    Expr cst_false;
    Expr cst_true;

    // context
    Vec<Expr> cond_stack;
    Var       sys_state;

    Vec<String>    inc_paths;
    AutoPtr<Scope> main_scope;

    int off;        ///< current offset in sourcefile
    SourceFile *sf; ///< current sourcefile

    //
    ErrorList error_list;
    NstrCor   str_cor;
};

extern Ip *ip;

#endif // IP_H

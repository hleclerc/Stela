#ifndef IP_H
#define IP_H

#include "../System/ErrorList.h"
#include "../System/NstrCor.h"
#include "Sourcefile.h"
#include "Scope.h"
#include <map>
class Class;
class Type;
class Expr;

/**
*/
class Ip {
public:
    struct CS { SourceFile *sf; int off; const char *reason; };

    Ip();
    ~Ip();

    Expr ret_error( String msg, bool warn = false, const char *file = 0, int line = 0 );
    void disp_error( String msg, bool warn = false, const char *file = 0, int line = 0 );
    ErrorList::Error &error_msg( String msg, bool warn = false, const char *file = 0, int line = 0 );

    Expr error_var();
    Expr void_var();

    NamedVarList *get_static_scope( String path );

    void add_inc_path( String inc_path );
    void import( String file );

    SourceFile *new_sf( String file );

    Expr make_Varargs( Vec<Expr> &v_args, Vec<int> &v_names );
    Expr make_Callable( Vec<Expr> &lst, Expr self );
    Expr make_type_var( Type *type );

    #define DECL_BT( T ) Type *type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT
    Type *type_ST;

    #define DECL_BT( T ) Class *class_##T;
    #include "DeclParmClass.h"
    #include "DeclBaseClass.h"
    #undef DECL_BT

    std::map<String,SourceFile>   sourcefiles;
    std::map<String,NamedVarList> static_scopes;
    Vec<String>                   inc_paths;

    ErrorList                     error_list;
    NstrCor                       str_cor;

    Expr                          sys_state;

    Scope                         main_scope;
    Scope                        *cur_scope;
};

extern Ip *ip;

#endif // IP_H

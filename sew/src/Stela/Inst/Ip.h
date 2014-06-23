#ifndef IP_H
#define IP_H

#include "../System/ErrorList.h"
#include "../System/NstrCor.h"
#include "Sourcefile.h"
#include "Scope.h"
#include <map>
class IpSnapshot;
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

    Vec<Expr> *get_static_vars( String path ); ///<
    Expr reg_var( int name, Expr var ); ///< in main scope

    void add_inc_path( String inc_path );
    void import( String file );

    SourceFile *new_sf( String file );

    Expr make_Varargs( Vec<Expr> &lst, const Vec<int> &names = Vec<int>() );
    Expr make_SurdefList( Vec<Expr> &surdefs, Expr self = Expr() );
    Expr make_type_var( Type *type );

    Type *ptr_for( Type *type );
    Type *make_Varargs_type( const Vec<Type *> &types, const Vec<int> &names, int o );
    Type *type_from_type_var( Expr var );

    #define DECL_BT( T ) Type *type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT
    Type *type_ST;
    Type *type_Ptr_SI32;
    int   ptr_size;

    #define DECL_BT( T ) Class *class_##T;
    #include "DeclParmClass.h"
    #include "DeclBaseClass.h"
    #undef DECL_BT

    std::map<String,SourceFile> sourcefiles;
    std::map<String,Vec<Expr> > static_vars;
    std::map<Type *,Type *>     ptr_map;
    Vec<String>                 inc_paths;
    NamedVarList                vars;

    ErrorList                   error_list;
    NstrCor                     str_cor;

    Expr                        sys_state;

    Scope                       main_scope;
    Scope                      *cur_scope;
    IpSnapshot                 *cur_ip_snapshot;
};

extern Ip *ip;

#endif // IP_H

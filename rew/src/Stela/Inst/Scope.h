#ifndef SCOPE_H
#define SCOPE_H

#include "../System/BinStreamReader.h"
#include "Var.h"
class SourceFile;
class Ip;

/**
*/
class Scope {
public:
    struct NamedVar {
        bool operator==( SI32 n ) const { return n == name; }
        SI32        name;
        Var         var;
        SourceFile *sf;
        SI32        off;
    };
    struct VecNamedVar {
        Var add( int name, Var var );
        bool contains( int name );
        void get( Vec<Var> &lst, int name );
        Var get( int name );
        Vec<NamedVar> data;
    };

    Scope( Scope *parent, String name, Ip *ip = 0 );

    void import( String file );
    Var  parse( const PI8 *tok );
    Var  parse( SourceFile *sf, const PI8 *tok, const char *reason ); ///< version that change sf in ip

    bool         do_not_execute_anything;
    VecNamedVar  local_scope;
    VecNamedVar *static_scope;
    const Type  *class_scope;
    String       path;
    Ip          *ip;

    int          base_size;
    int          base_alig;

    Scope       *parent;
    Var          self;
    Var          cont;
    bool         method;

protected:
    enum ApplyMode { APPLY_MODE_STD, APPLY_MODE_PARTIAL_INST, APPLY_MODE_NEW };
    friend class Class;
    friend class Type;
    friend class Var;
    friend class Def;
    friend class Ip;

    int  read_nstring( BinStreamReader &bin );
    Var  reg_var( int name, Var var, bool static_scope = false );
    Var  find_first_var( int name );
    void find_var_clist( Vec<Var> &lst, int name );
    Var  find_var( int name );
    Var  parse_CALLABLE( BinStreamReader bin, Type *type );
    Var  apply( Var f, int nu = 0, Var *u_args = 0, int nn = 0, int *n_name = 0, Var *n_args = 0, ApplyMode am = APPLY_MODE_STD );
    Var  get_attr( Var s, int name );
    Var  get_attr_rec( Var self, int name );
    void get_attr_rec( Vec<Var> &res, Var self, int name );
    Var copy( Var &var );

    template<class OP> Var parse_una( BinStreamReader bin, OP op );
    template<class OP> Var parse_bin( BinStreamReader bin, OP op );
    template<class OP> Type *type_promote( Type *ta, Type *tb, OP op );
    template<class OP> Type *type_promote( Type *ta, OP op );

    #define DECL_IR_TOK( N ) Var parse_##N( BinStreamReader bin );
    #include "../Ir/Decl.h"
    #undef DECL_IR_TOK
};

#endif // SCOPE_H

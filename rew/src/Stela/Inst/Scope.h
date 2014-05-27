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
        SI32 name;
        Var  var;
    };
    struct VecNamedVar {
        Var add( int name, Var var );
        bool contains( int name );
        Var get( int name );
        Vec<NamedVar> data;
    };

    Scope( Scope *parent, String name, Ip *ip = 0 );

    void import( String file );
    Var  parse( const PI8 *tok );

    bool         do_not_execute_anything;
    VecNamedVar  local_scope;
    VecNamedVar *static_scope;
    String       path;
    Ip          *ip;

    Scope       *parent;

protected:
    int read_nstring( BinStreamReader &bin );
    Var reg_var( int name, Var var, bool static_scope = false );
    Var find_first_var( int name );
    Var find_var( int name );

    #define DECL_IR_TOK( N ) Var parse_##N( BinStreamReader bin );
    #include "../Ir/Decl.h"
    #undef DECL_IR_TOK
};

#endif // SCOPE_H

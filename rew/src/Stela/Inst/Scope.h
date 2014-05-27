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
        SI32 name;
        Expr inst;
    };

    Scope( Scope *parent, String name, Ip *ip );

    void import( String file );
    Var  parse( const PI8 *tok );

    bool           do_not_execute_anything;
    Vec<NamedVar>  local_scope;
    Vec<NamedVar> *static_scope;
    String         path;
    Ip            *ip;

protected:
    #define DECL_IR_TOK( N ) Var parse_##N( BinStreamReader bin );
    #include "../Ir/Decl.h"
    #undef DECL_IR_TOK
};

#endif // SCOPE_H

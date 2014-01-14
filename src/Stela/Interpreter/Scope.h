#ifndef SCOPE_H
#define SCOPE_H

#include "../System/BinStreamReader.h"
#include "Var.h"
class Interpreter;
class SourceFile;

/**
*/
class Scope {
public:
    Scope( Interpreter *ip, Scope *parent, Scope *caller = 0 );

    Var parse( const Var *sf, const PI8 *tok );

protected:
    friend class Interpreter;

    #define DECL_IR_TOK( N ) Var parse_##N( const Var *sf, int off, BinStreamReader bin ); ///< parse a given IR_TOK
    #include "../Ir/Decl.h"
    #undef DECL_IR_TOK

    int read_nstring( const SourceFile *sf, BinStreamReader &bin );

    Interpreter *ip;
    Scope       *parent; ///< "accessible" scope, i.e. that can be read to find variables
    Scope       *caller; ///< caller scope, if this serves as the body of a function

    bool         do_not_execute_anything;
    Var         *instantiated_from_sf;
    int          instantiated_from_off;
};

#endif // SCOPE_H

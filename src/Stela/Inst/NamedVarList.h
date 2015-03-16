#ifndef NAMEDVARLIST_H
#define NAMEDVARLIST_H

#include "Inst.h"
class SourceFile;
/**
*/
class NamedVarList {
public:
    struct NamedVar {
        bool operator==( SI32 n ) const { return n == name; }
        SI32        name;
        Expr        expr;
        SourceFile *sf;
        SI32        off;
    };

    Expr add( int name, Expr expr );
    bool contains( int name );
    void get( Vec<Expr> &lst, int name );
    Expr get( int name );

    Vec<NamedVar> data;
};

#endif // NAMEDVARLIST_H

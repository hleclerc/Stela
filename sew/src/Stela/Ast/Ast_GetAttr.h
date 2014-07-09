#ifndef STELA_AST_GetAttr_H
#define STELA_AST_GetAttr_H

#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_GetAttr : public Ast {
public:
    Ast_GetAttr( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

    String       name;
    AutoPtr<Ast> obj;
    bool         ptr;
    bool         ask;
    bool         ddo;
};

#endif // STELA_AST_GetAttr_H


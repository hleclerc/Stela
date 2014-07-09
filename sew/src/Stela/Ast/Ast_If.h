#ifndef STELA_AST_If_H
#define STELA_AST_If_H

#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_If : public Ast {
public:
    Ast_If( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

    AutoPtr<Ast> cond;
    AutoPtr<Ast> ok;
    AutoPtr<Ast> ko;
};

#endif // STELA_AST_If_H


#ifndef STELA_AST_While_H
#define STELA_AST_While_H

#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_While : public Ast {
public:
    Ast_While( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

    AutoPtr<Ast> ok;
    AutoPtr<Ast> ko;
};

#endif // STELA_AST_While_H


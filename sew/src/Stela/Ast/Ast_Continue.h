#ifndef STELA_AST_Continue_H
#define STELA_AST_Continue_H

#include "Ast.h"

/**
*/
class Ast_Continue : public Ast {
public:
    Ast_Continue( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;
    int n;
};

#endif // STELA_AST_Continue_H


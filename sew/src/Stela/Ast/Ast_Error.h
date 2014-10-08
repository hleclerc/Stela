#ifndef AST_ERROR_H
#define AST_ERROR_H

#include "Ast.h"

/**
*/
class Ast_Error : public Ast {
public:
    Ast_Error();
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;
};

#endif // AST_ERROR_H

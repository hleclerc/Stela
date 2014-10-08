#ifndef AST_SYMBOL_H
#define AST_SYMBOL_H

#include "Ast.h"

/**
*/
class Ast_Symbol : public Ast {
public:
    Ast_Symbol( String str );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

    String str;
};

#endif // AST_SYMBOL_H

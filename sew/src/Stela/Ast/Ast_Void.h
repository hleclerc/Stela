#ifndef STELA_AST_Void_H
#define STELA_AST_Void_H

#include "Ast.h"

/**
*/
class Ast_Void : public Ast {
public:
    Ast_Void( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;
};

#endif // STELA_AST_Void_H


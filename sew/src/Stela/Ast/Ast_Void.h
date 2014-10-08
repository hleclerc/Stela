#ifndef STELA_AST_Void_H
#define STELA_AST_Void_H

#include "Ast.h"

/**
*/
class Ast_Void : public Ast {
public:
    Ast_Void( int off = -1 );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    virtual PI8  _tok_number() const;
};

#endif // STELA_AST_Void_H


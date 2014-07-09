#ifndef AST_VARIABLE_H
#define AST_VARIABLE_H

#include "Ast.h"

/**
*/
class Ast_Variable : public Ast {
public:
    Ast_Variable( int off, String str );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    virtual void _get_info( AstWriter *aw ) const;
    virtual PI8  _tok_number() const;

    String str;
};

#endif // AST_VARIABLE_H

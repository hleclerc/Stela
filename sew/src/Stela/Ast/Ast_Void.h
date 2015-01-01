#ifndef STELA_AST_Void_H
#define STELA_AST_Void_H

#include "Ast.h"

/**
*/
class Ast_Void : public Ast {
public:
    Ast_Void( const char *src = 0, int off = -1 );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual PI8  _tok_number() const;
};

#endif // STELA_AST_Void_H


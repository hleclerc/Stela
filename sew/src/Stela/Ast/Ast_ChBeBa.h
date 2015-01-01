#ifndef STELA_AST_ChBeBa_H
#define STELA_AST_ChBeBa_H

#include "Ast_Call.h"

/**
*/
class Ast_ChBeBa : public Ast_Call {
public:
    Ast_ChBeBa( const char *src, int off );
    virtual void write_name( Stream &os ) const { os << "chbeba"; }

protected:
    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual PI8  _tok_number() const;
};

#endif // STELA_AST_ChBeBa_H


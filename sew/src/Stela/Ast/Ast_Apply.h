#ifndef STELA_AST_Apply_H
#define STELA_AST_Apply_H

#include "Ast_Call.h"

/**
*/
class Ast_Apply : public Ast_Call {
public:
    Ast_Apply( int off );

    virtual void write_name( Stream &os ) const { os << "apply"; }

protected:
    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual PI8  _tok_number() const;
};

#endif // STELA_AST_Apply_H


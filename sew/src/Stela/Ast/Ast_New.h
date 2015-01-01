#ifndef STELA_AST_New_H
#define STELA_AST_New_H

#include "Ast_Call.h"

/**
*/
class Ast_New : public Ast_Call {
public:
    Ast_New( const char *src, int off );
    virtual void write_name( Stream &os ) const { os << "new"; }

protected:
    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual PI8  _tok_number() const;
};

#endif // STELA_AST_New_H


#ifndef STELA_AST_Select_H
#define STELA_AST_Select_H

#include "Ast_Call.h"

/**
*/
class Ast_Select : public Ast_Call {
public:
    Ast_Select( int off );
    virtual void write_name( Stream &os ) const { os << "select"; }
};

#endif // STELA_AST_Select_H


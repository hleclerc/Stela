#ifndef AST_SURDEFLIST_H
#define AST_SURDEFLIST_H

#include "Ast.h"

/**
*/
class Ast_SurdefList : public Ast {
public:
    Ast_SurdefList( Vec<Past> lst );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

    Vec<Past> lst;
};

#endif // AST_SURDEFLIST_H

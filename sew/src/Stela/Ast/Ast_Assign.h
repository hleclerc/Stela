#ifndef STELA_AST_Assign_H
#define STELA_AST_Assign_H

#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_Assign : public Ast {
public:
    Ast_Assign( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

    String       name;
    AutoPtr<Ast> val;
    bool         stat;
    bool         cons;
    bool         type; ///< ... ~= ...
    bool         ref;
};

#endif // STELA_AST_Assign_H


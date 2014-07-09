#ifndef STELA_AST_Call_H
#define STELA_AST_Call_H

#include "../System/SplittedVec.h"
#include "../System/AutoPtr.h"
#include "../System/Vec.h"
#include "Ast.h"

/**
*/
class Ast_Call : public Ast {
public:
    Ast_Call( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;
    virtual void write_name( Stream &os ) const = 0;
    AutoPtr<Ast>                 f;
    SplittedVec<AutoPtr<Ast>,4 > args;
    Vec<String>                  names;
};

#endif // STELA_AST_Call_H


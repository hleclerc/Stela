#ifndef STELA_AST_Class_H
#define STELA_AST_Class_H

#include "Ast_Callable.h"

/**
*/
class Ast_Class : public Ast_Callable {
public:
    Ast_Class( int off );
    virtual void write_callable_type( Stream &os ) const { os << "class"; }

    SplittedVec<AutoPtr<Ast>,2> inheritance;
};

#endif // STELA_AST_Class_H


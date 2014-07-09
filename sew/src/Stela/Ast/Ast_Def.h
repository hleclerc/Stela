#ifndef STELA_AST_Def_H
#define STELA_AST_Def_H

#include "Ast_Callable.h"
#include "Ast_Apply.h"

/**
*/
class Ast_Def : public Ast_Callable {
public:
    struct StartsWith_Item {
        String                      attr;
        SplittedVec<AutoPtr<Ast>,2> args;
        Vec<String>                 names;
    };

    Ast_Def( int off );
    virtual void write_callable_type( Stream &os ) const { os << "def"; }

    AutoPtr<Ast>                   return_type;
    String                         get;
    String                         set;
    String                         sop;
    SplittedVec<StartsWith_Item,2> starts_with;
};

#endif // STELA_AST_Def_H


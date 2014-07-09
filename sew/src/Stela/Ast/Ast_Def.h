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

    virtual void get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void write_callable_type( Stream &os ) const { os << "def"; }

protected:
    friend class AstMaker;

    virtual void _get_info( AstWriter *aw ) const;
    virtual PI8  _tok_number() const;
    virtual int  _spec_flags() const;

    AutoPtr<Ast>                   return_type;
    String                         get;
    String                         set;
    String                         sop;
    SplittedVec<StartsWith_Item,2> starts_with;
};

#endif // STELA_AST_Def_H


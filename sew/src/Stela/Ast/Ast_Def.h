#ifndef STELA_AST_Def_H
#define STELA_AST_Def_H

#include "Ast_Callable.h"
#include "Ast_Apply.h"
class Ast_Class;

/**
*/
class Ast_Def : public Ast_Callable {
public:
    struct StartsWith_Item {
        String              attr;
        SplittedVec<Past,2> args;
        Vec<String>         names;
    };

    Ast_Def( const char *src, int off );

    virtual void write_callable_type( Stream &os ) const { os << "def"; }

protected:
    friend class AstMaker;

    virtual void _get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;
    virtual int  _spec_flags() const;

    Past                           return_type;
    String                         get;
    String                         set;
    String                         sop;
    SplittedVec<StartsWith_Item,2> starts_with;
    Ast_Class                     *method;
};

#endif // STELA_AST_Def_H


#ifndef STELA_AST_If_H
#define STELA_AST_If_H

#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_If : public Ast {
public:
    Ast_If( const char *src, int off );

    virtual void get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    friend class AstMaker;

    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;

    Past cond;
    Past ok;
    Past ko;
};

#endif // STELA_AST_If_H


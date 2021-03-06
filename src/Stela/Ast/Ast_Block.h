#ifndef STELA_AST_Block_H
#define STELA_AST_Block_H

#include "../System/SplittedVec.h"
#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_Block : public Ast {
public:
    Ast_Block( const char *src, int off, bool want_ret );

    virtual void get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void prep_get_potentially_needed_ext_vars( std::set<String> &avail ) const;
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    friend class AstMaker;

    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;

    SplittedVec<Past,8> lst;
    bool want_ret;
};

#endif // STELA_AST_Block_H


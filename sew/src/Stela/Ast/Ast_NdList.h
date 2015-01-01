#ifndef STELA_AST_NdList_H
#define STELA_AST_NdList_H

#include "../System/SplittedVec.h"
#include "../System/AutoPtr.h"
#include "../System/Vec.h"
#include "Ast.h"

/**
*/
class Ast_NdList : public Ast {
public:
    Ast_NdList( const char *src, int off );

    virtual void get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    friend class AstMaker;

    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;

    int                         nb_dim;
    Vec<int>                    sizes; ///<
    SplittedVec<Past,8> lst;
};

#endif // STELA_AST_NdList_H


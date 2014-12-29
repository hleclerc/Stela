#ifndef STELA_AST_Primitive_H
#define STELA_AST_Primitive_H

#include "../System/SplittedVec.h"
#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_Primitive : public Ast {
public:
    Ast_Primitive( int off, int tok_number );

    virtual void get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;
    
    friend class AstMaker;
    
    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;

    SplittedVec<Past,4> args;
    Vec<String>         names;
    int tok_number;
};

#endif // STELA_AST_Primitive_H


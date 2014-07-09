#ifndef STELA_AST_Primitive_H
#define STELA_AST_Primitive_H

#include "../System/SplittedVec.h"
#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_Primitive : public Ast {
public:
    Ast_Primitive( int off );

    virtual void get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;
    
protected:
    friend class AstMaker;
    
    virtual void _get_info( AstWriter *aw ) const;
    virtual PI8  _tok_number() const;

    SplittedVec<AutoPtr<Ast>,4 > args;
    int tok_number;
};

#endif // STELA_AST_Primitive_H


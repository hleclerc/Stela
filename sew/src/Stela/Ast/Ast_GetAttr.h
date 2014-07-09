#ifndef STELA_AST_GetAttr_H
#define STELA_AST_GetAttr_H

#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_GetAttr : public Ast {
public:
    Ast_GetAttr( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    friend class AstMaker;

    virtual void _get_info( AstWriter *aw ) const;
    virtual PI8  _tok_number() const;

    String       name;
    AutoPtr<Ast> obj;
    bool         ptr;
    bool         ask;
    bool         ddo;
};

#endif // STELA_AST_GetAttr_H


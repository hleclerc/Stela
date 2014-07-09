#ifndef STELA_AST_Return_H
#define STELA_AST_Return_H

#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_Return : public Ast {
public:
    Ast_Return( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    friend class AstMaker;

    virtual void _get_info( AstWriter *aw ) const;
    virtual PI8  _tok_number() const;

    AutoPtr<Ast> val;
};

#endif // STELA_AST_Return_H


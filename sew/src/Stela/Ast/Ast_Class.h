#ifndef STELA_AST_Class_H
#define STELA_AST_Class_H

#include "Ast_Callable.h"

/**
*/
class Ast_Class : public Ast_Callable {
public:
    Ast_Class( int off );
    virtual void write_callable_type( Stream &os ) const { os << "class"; }

protected:
    friend class AstMaker;

    virtual void _get_info( AstWriter *aw ) const;
    virtual PI8  _tok_number() const;
    virtual int  _spec_flags() const;

    SplittedVec<AutoPtr<Ast>,2> inheritance;
};

#endif // STELA_AST_Class_H


#ifndef STELA_AST_Callable_H
#define STELA_AST_Callable_H

#include "../System/SplittedVec.h"
#include "../System/AutoPtr.h"
#include "../System/Vec.h"
#include "Ast.h"

/**
*/
class Ast_Callable : public Ast {
public:
    typedef SplittedVec<AutoPtr<Ast>,4> DVT;

    Ast_Callable( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;
    virtual void write_callable_type( Stream &os ) const = 0;

protected:
    friend class AstMaker;

    virtual void _get_info( AstWriter *aw ) const;
    virtual int  _spec_flags() const = 0;

    String            name;
    bool              self_as_arg;
    bool              abstract;
    bool              varargs;
    AutoPtr<Ast>      pertinence;
    AutoPtr<Ast>      condition;
    int               def_pert_num;
    int               def_pert_den;
    Vec<String>       arguments;
    Vec<Vec<String> > arg_constraints; ///< same size than arguments
    DVT               default_values;
    AutoPtr<Ast>      block;
};

#endif // STELA_AST_Callable_H


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
    Ast_NdList( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    friend class AstMaker;

    virtual void _get_info( AstWriter *aw ) const;
    virtual PI8  _tok_number() const;

    int                         nb_dim;
    Vec<int>                    sizes; ///<
    SplittedVec<AutoPtr<Ast>,8> lst;
};

#endif // STELA_AST_NdList_H


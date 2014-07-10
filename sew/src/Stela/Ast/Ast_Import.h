#ifndef STELA_AST_Import_H
#define STELA_AST_Import_H

#include "../System/Vec.h"
#include "Ast.h"

/**
*/
class Ast_Import : public Ast {
public:
    Ast_Import( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    friend class AstMaker;

    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;

    Vec<String> files;
};

#endif // STELA_AST_Import_H


#ifndef STELA_AST_AndOrOr_H
#define STELA_AST_AndOrOr_H

#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_AndOrOr : public Ast {
public:
    Ast_AndOrOr( int off, bool want_and );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;
    
protected:
    friend class AstMaker;
    
    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;

    Past args[ 2 ];
    bool want_and;
};

#endif // STELA_AST_AndOrOr_H


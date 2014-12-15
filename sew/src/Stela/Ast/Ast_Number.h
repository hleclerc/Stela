#ifndef STELA_AST_Number_H
#define STELA_AST_Number_H

#include "Ast.h"

/**
*/
class Ast_Number : public Ast {
public:
    Ast_Number( int off );
    Ast_Number( int off, bool val );
    Ast_Number( int off, String str );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    friend class AstMaker;

    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;

    bool l; ///< long
    bool p; ///< pointer
    bool b; ///< boolean
    String str;
};

#endif // STELA_AST_Number_H


#ifndef STELA_AST_String_H
#define STELA_AST_String_H

#include "Ast.h"

/**
*/
class Ast_String : public Ast {
public:
    Ast_String( int off, String str );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;

    String str;
};

#endif // STELA_AST_String_H


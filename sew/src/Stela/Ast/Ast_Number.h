#ifndef STELA_AST_Number_H
#define STELA_AST_Number_H

#include "Ast.h"

/**
*/
class Ast_Number : public Ast {
public:
    Ast_Number( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

    bool l; ///< long
    bool p; ///< pointer
    String str;
};

#endif // STELA_AST_Number_H


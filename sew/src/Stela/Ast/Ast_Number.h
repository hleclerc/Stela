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

    template<class T>
    static int size_for_CUInt( T val ) {
        ST res = 1;
        for( ; val >= 128; val /= 128 )
            ++res;
        return res;
    }

    template<class T>
    static void write_CUInt( PI8 *buf, T val ) {
        for( ; val >= 128; val /= 128 )
            *( buf++ ) = 128 + val % 128;
        *( buf++ ) = val;
    }

protected:
    friend class AstMaker;

    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;

    bool l; ///< long
    bool p; ///< pointer
    bool b; ///< boolean
    String str;
};


#endif // STELA_AST_Number_H


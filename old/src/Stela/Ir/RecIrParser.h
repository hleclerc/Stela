#ifndef RECIRPARSER_H
#define RECIRPARSER_H

#include "../System/BinStreamReader.h"

/**
*/
class RecIrParser {
public:
    void parse( const PI8 *bin );

protected:
    #define DECL_IR_TOK( N ) virtual void parse_##N( BinStreamReader bin );
    #include "../Ir/Decl.h"
    #undef DECL_IR_TOK

    void _parse_APPLY( BinStreamReader bin, bool ret );
    void _parse_CALLABLE( BinStreamReader bin, bool def );
    void _parse_GET_ATTR( BinStreamReader bin );
};

#endif // RECIRPARSER_H

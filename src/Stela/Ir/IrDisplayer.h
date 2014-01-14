#ifndef IRDISPLAYER_H
#define IRDISPLAYER_H

#include "../System/Stream.h"
class BinStreamReader;

/**
*/
class IrDisplayer {
public:
    IrDisplayer( const PI8 *cor, const PI8 *ir, bool rec = true, std::string sp = "  " );
    IrDisplayer( const PI8 *bin, bool rec = true, std::string sp = "  " );
    void write_to_stream( Stream &os ) const;

protected:
    #define DECL_IR_TOK( N ) void parse_##N( Stream &os, BinStreamReader bin ) const; ///< parse a given IR_TOK
    #include "../Ir/Decl.h"
    #undef DECL_IR_TOK

    std::string nstring( int n ) const;
    int read_nstring( BinStreamReader &bin ) const;
    void _parse_CALLABLE( Stream &os, BinStreamReader bin, bool def ) const;
    void _parse_APPLY( Stream &os, BinStreamReader bin, const char *type ) const;

    std::string sp;
    const PI8 *cor;
    const PI8 *ir;
    bool rec;

    mutable int off;
};

#endif // IRDISPLAYER_H

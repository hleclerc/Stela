#ifndef TYPE_H
#define TYPE_H

#include "Inst.h"
class SourceFile;
class Class;

/**
*/
class Type {
public:
    struct Attr {
        bool dyn() const { return offset >= 0; }
        bool sta() const { return offset <  0; }

        int  offset; ///< in bits
        int  name;
        Expr var;

        SourceFile *sf;
        int off; ///< on sourcefile
    };

    Type( Class *orig );

    void write_to_stream( Stream &os );
    void write_to_stream( Stream &os, void *data, int len );

    int size();
    int sb();

    void parse();

    Vec<Expr> parameters;
    Class    *orig;
    bool      aryth;
    int       _len;
    Vec<Attr> _attributes; ///< dynamic and static attributes
};

#endif // TYPE_H

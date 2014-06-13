#ifndef TYPE_H
#define TYPE_H

#include "Inst.h"
class SourceFile;
class Class;

/**
*/
class Type {
public:
    Type( Class *orig );

    void write_to_stream( Stream &os );
    void write_to_stream( Stream &os, void *data, int len );

    Expr size( Inst *inst ); ///< size in bits
    int  size(); ///< size in bits (-1 if depend on the actual data)
    int  pod();  ///< plain old data
    int  sb();   ///< size in bytes (-1 if depend on the actual data)

    Vec<Expr> parameters;
    Class    *orig;
    bool      aryth;
    int       _len;
    int       _pod;
};

#endif // TYPE_H

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
    void parse();

    int  size(); ///< size in bits
    int  pod();  ///< plain old data
    int  sb();   ///< size in bytes

    Vec<Expr>   parameters;
    Class      *orig;
    bool        aryth;
    bool        _parsed;
    int         _len;
    int         _pod;

    Vec<Type *> attr_ptr_types;
};

#endif // TYPE_H

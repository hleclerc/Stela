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
        int  off; ///< <=0 -> static
        Expr val; ///< flags may be == Inst::PART_INST
        int  name;
    };
    Type( Class *orig );

    void write_to_stream( Stream &os );
    void write_to_stream( Stream &os, void *data, int len );
    void parse();

    int  size(); ///< size in bits
    int  alig(); ///< alignement in bits
    int  pod();  ///< plain old data
    int  sb();   ///< size in bytes

    Vec<Expr>   parameters;
    Class      *orig;
    bool        aryth;
    bool        _parsed;
    int         _len;
    int         _ali;
    int         _pod;

    Vec<Attr>   attributes;
    Vec<Type *> ancestors;
};

#endif // TYPE_H

#ifndef TYPE_H
#define TYPE_H

#include "Expr.h"
class Class;

/**
*/
class Type {
public:
    Type( Class *orig );

    void write_to_stream( Stream &os );
    void write_to_stream( Stream &os, void *data, int len );

    int size();
    int sb();

    void parse();

    Class *orig;
    bool   aryth;
    int    _len;
};

#endif // TYPE_H

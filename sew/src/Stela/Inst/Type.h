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

    void parse();

    Class *orig;
    int    _len;
};

#endif // TYPE_H

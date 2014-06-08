#ifndef TYPE_H
#define TYPE_H

#include "Expr.h"

/**
*/
class Type {
public:
    Type();

    void write_to_stream( Stream &os );
    void write_to_stream( Stream &os, void *data, int len );

    int size();

    void parse();

    int _len;
};

#endif // TYPE_H

#ifndef TYPE_H
#define TYPE_H

#include "Var.h"

/**
*/
class Type {
public:
    Type( int name );

    void write_to_stream( Stream &os ) const;
    int size();

    virtual void parse();

    int name;
    int _size;
};

#endif // TYPE_H

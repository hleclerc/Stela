#ifndef CLASS_H
#define CLASS_H

#include "Type.h"

/**
*/
class Class {
public:
    Class();

    void write_to_stream( Stream &os ) const;

    int name;
};

#endif // CLASS_H

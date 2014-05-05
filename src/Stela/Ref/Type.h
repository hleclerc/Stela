#ifndef TYPE_H
#define TYPE_H

#include "../System/Stream.h"
/**
*/
class Type {
public:
    Type();

    int size_in_bits() const;
    int size_in_bytes() const;

    void write_to_stream( Stream &os ) const;
};

#endif // TYPE_H

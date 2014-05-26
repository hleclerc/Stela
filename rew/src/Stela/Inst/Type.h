#ifndef TYPE_H
#define TYPE_H

#include "../System/Stream.h"

/**
*/
class Type {
public:
    Type( int name = -1, int len = -1 );

    void write_to_stream( Stream &os ) const;
    void write_C_decl( Stream &out ) const;

    int pod() const;
    int size() const;
    virtual void parse() const;

    int name;
    mutable int _len;
    mutable int _pod;
};

#endif // TYPE_H

#ifndef TYPE_H
#define TYPE_H

#include "../System/Stream.h"

/**
*/
class Type {
public:
    Type( int name = -1, int len = -1 );

    void write_to_stream( Stream &os ) const;

    int pod();
    int size();
    virtual void parse();

    int name;
    int _len;
    int _pod;
};

#endif // TYPE_H

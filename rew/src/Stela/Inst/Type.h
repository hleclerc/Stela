#ifndef TYPE_H
#define TYPE_H

#include "../System/Stream.h"
#include "Var.h"
class Class;

/**
*/
class Type {
public:
    Type( int name = -1 );

    void write_to_stream( Stream &os ) const;
    void write_C_decl( Stream &out ) const;

    int pod() const;
    int size() const;
    virtual void parse() const;

    int         name;
    Class      *orig;
    mutable int _len;
    mutable int _pod;

    Vec<Var> parameters; ///< template parameters
};

#endif // TYPE_H

#ifndef PREF_H
#define PREF_H

#include "Ref.h"
class Interpreter;

/**
*/
struct PRef : ObjectWithCptUse {
    enum {
        CONST = 1 ///< this variable cannot be changed (and it's definitive)
    };

    PRef( Interpreter *ip ) : ip( ip ), flags( 0 ) {}
    void write_to_stream( Stream &os ) const { os << ptr; }

    Ptr<Ref>     ptr;
    Interpreter *ip;
    int          flags;
};

#endif // PREF_H

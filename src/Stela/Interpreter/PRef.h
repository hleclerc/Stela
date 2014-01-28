#ifndef PREF_H
#define PREF_H

#include "Ref.h"
class Interpreter;

/**
  Pointer on a ref object
*/
struct PRef : ObjectWithCptUse {
    enum {
        CONST = 1 ///< this variable cannot be changed (and it's definitive)
    };

    PRef() : flags( 0 ) {}
    void write_to_stream( Stream &os ) const { os << ptr; }
    Expr expr() const { return ptr->expr(); }

    bool is_const() const { return flags & CONST; }

    Ptr<Ref>     ptr;
    int          flags;
};

#endif // PREF_H

#ifndef PREF_H
#define PREF_H

#include "Ref.h"
class Interpreter;
class Var;

/**
  Pointer on a ref object
*/
struct PRef : ObjectWithCptUse {
    enum {
        CONST = 1 ///< this variable cannot be changed (and it's definitive)
    };
    struct VRF {
        Var var;
        int off; ///< offset in bits
    };

    PRef( Ref *ref ) : ptr( ref ), refs( 0 ), flags( 0 ) {}
    PRef() : refs( 0 ), flags( 0 ) {}
    ~PRef();

    void write_to_stream( Stream &os ) const { os << ptr; }
    Expr expr() const { return ptr ? ptr->expr() : Expr(); }

    void add_ref( int offset, const Var &var );
    Var  get_ref( int offset );
    VRF *get_vrf( int offset );

    bool is_const() const { return flags & CONST; }

    Ptr<Ref>     ptr;
    Vec<VRF>    *refs; ///< holded references
    int          flags;
};

#endif // PREF_H

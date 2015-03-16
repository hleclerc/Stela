#include "Uninitialized.h"

/**
*/
struct Uninitialized : Inst {
    virtual void write_dot( Stream &os ) const { os << "Uninitialized"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Uninitialized; }
    virtual Type *type() { return 0; }
    virtual bool uninitialized() const { return true; }
    virtual void set( const Expr &obj, const BoolOpSeq &cond ) {
    }
};

Inst *uninitialized() {
    Uninitialized *res = new Uninitialized;
    return res;
}


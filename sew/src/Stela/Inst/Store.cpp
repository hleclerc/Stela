#include "Store.h"

/**
*/
struct Store : Inst {
    virtual void write_dot( Stream &os ) { os << "Store"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Store; }
    virtual Type *type() { return inp[ 0 ]->type(); }
};

Expr store( Expr ptr, Expr val ) {
    Store *res = new Store();
    res->add_inp( ptr );
    res->add_inp( val );
    return res;
}


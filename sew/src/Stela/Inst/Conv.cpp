#include "Conv.h"
#include "Type.h"

/**
*/
struct Conv : Inst {
    Conv( Type *dst ) : dst( dst ) {}
    virtual void write_dot( Stream &os ) { os << "Conv[" << *dst << "]"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Conv( dst ); }
    virtual Type *type() { return dst; }
    Type *dst;
};

Expr conv( Type *dst, Expr inp ) {
    if ( dst == inp->type() )
        return inp;

    Conv *res = new Conv( dst );
    res->add_inp( inp );
    return res;
}


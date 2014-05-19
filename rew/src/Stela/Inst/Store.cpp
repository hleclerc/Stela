#include "Store.h"

/**
*/
class Store : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "store";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Store;
    }
    virtual int size() const {
        return 0;
    }
};

Expr store( Expr ptr, Expr val ) {
    Store *res = new Store;
    res->add_inp( ptr );
    res->add_inp( val );
    return res;
}

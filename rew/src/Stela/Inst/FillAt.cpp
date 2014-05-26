#include "FillAt.h"


/**
*/
class FillAt : public Inst {
public:
    virtual void write_dot( Stream &os ) const { os << "fill_at"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new FillAt; }
    virtual int size() const { return inp[ 0 ]->size(); }
};

Expr fill_at( Expr src, Expr val, Expr off ) {
    FillAt *res = new FillAt;
    res->add_inp( src );
    res->add_inp( val );
    res->add_inp( off );
    return res;
}

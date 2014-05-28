#include "FillAt.h"


/**
*/
class FillAt : public Inst {
public:
    virtual void write_dot( Stream &os ) const { os << "fill_at"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new FillAt; }
    virtual int size() const { return inp[ 0 ]->size(); }
};

Expr fill_at( Expr src, Expr val, Type *off_type, Expr off ) {
    SI32 off_val;
    if ( off->get_val( off_val, off_type ) ) {
        if ( off_val == 0 and src->size() == val->size() )
            return val;
    }

    FillAt *res = new FillAt;
    res->add_inp( src );
    res->add_inp( val );
    res->add_inp( off );
    return res;
}

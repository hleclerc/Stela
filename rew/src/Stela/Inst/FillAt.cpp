#include "FillAt.h"
#include "Cst.h"


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
        // complete fill ?
        if ( off_val == 0 and src->size() == val->size() )
            return val;

        //
        PI8 data[ src->sb() ];
        int pb = off_val + val->size();
        if ( src->get_val( data, off_val ) and
             val->get_val( data, val->size(), 0, off_val ) and
             src->get_val( data, src->size() - pb, pb, pb ) ) {
            // TODO: get_val with unknown
            return cst( src->size(), data );
        }
    }



    FillAt *res = new FillAt;
    res->add_inp( src );
    res->add_inp( val );
    res->add_inp( off );
    return res;
}

#include "Slice.h"
#include "Cst.h"
#include "Ip.h"

/**
*/
class Slice : public Inst {
public:
    Slice( Type *off_type, int len ) : off_type( off_type ), len( len ) { }
    virtual void write_dot( Stream &os ) const { os << "slice[" << len << "]"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Slice( off_type, len ); }
    virtual int size() const { return len; }

    Type *off_type;
    int len;
};

Expr slice( Expr ptr, Type *off_type, Expr off, int len ) {
    // known offset value ?
    SI32 off_val;
    if ( off->get_val( off_val, off_type ) ) {
        if ( Expr res = ptr->_simp_slice( off_val, len ) )
            return res;
    }

    Slice *res = new Slice( off_type, len );
    res->add_inp( ptr );
    res->add_inp( off );
    return res;
}

Expr slice( Expr ptr, int off, int len ) {
    if ( Expr res = ptr->_simp_slice( off, len ) )
        return res;
    return slice( ptr, &ip->type_SI32, cst( 32, &off ), len );
}



#include "Slice.h"
#include "Ip.h"

/**
*/
class Slice : public Inst {
public:
    Slice( int len ) : len( len ) { }
    virtual void write_dot( Stream &os ) const { os << "slice[" << len << "]"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Slice( len ); }
    virtual int size() const { return len; }

    int len;
};

Expr slice( Expr ptr, Expr off, int len ) {
    Slice *res = new Slice( len );
    res->add_inp( ptr );
    res->add_inp( off );
    return res;
}

#include "GetNout.h"

/**
*/
class GetNout : public Inst {
public:
    GetNout( int nout ) : nout( nout ) {}
    virtual void write_dot( Stream &os ) const { os << "get_nout_" << nout; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new GetNout( nout ); }
    virtual int size() const { return inp[ 0 ]->size_out( nout ); }
    int nout;
};

Expr get_nout( Expr inp, int nout ) {
    GetNout *res = new GetNout( nout );
    res->add_inp( inp );
    return res;
}

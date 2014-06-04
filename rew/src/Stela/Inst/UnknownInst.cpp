#include "UnknownInst.h"

/**
*/
class UnknownInst : public Inst {
public:
    UnknownInst( int len, int cpt ) : len( len ), cpt( cpt ) {}
    virtual void write_dot( Stream &os ) const { os << "unknown_" << cpt; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new UnknownInst( len, cpt ); }
    virtual int size() const { return len; }



    int len;
    int cpt;
};

Expr unknown_inst( int len, int cpt ) {
    return new UnknownInst( len, cpt );
}

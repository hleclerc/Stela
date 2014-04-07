#include "Unknown.h"
#include "Inst_.h"

class Unknown : public Inst_<1,0> {
public:
    virtual int size_in_bits( int nout ) const { return _size_in_bits; }
    virtual void write_dot( Stream &os ) const { os << "unknown_" << cpt; }
    virtual void apply( InstVisitor &visitor ) const { ERROR( "..." ); }
    virtual int inst_id() const { return Inst::Id_Unknown; }

    int _size_in_bits, cpt;
};

Expr unknown_inst( int size_in_bits, int cpt ) {
    Unknown *res = new Unknown;
    res->_size_in_bits = size_in_bits;
    res->cpt = cpt;
    return Expr( res, 0 );
}

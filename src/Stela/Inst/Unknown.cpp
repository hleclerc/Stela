#include "Unknown.h"
#include "Inst_.h"

class Unknown : public Inst_<1,0> {
public:
    virtual int size_in_bits( int nout ) const { return _size_in_bits; }
    virtual void write_dot( Stream &os ) const { os << "unknown"; }
    virtual void apply( InstVisitor &visitor ) const { ERROR( "..." ); }
    virtual int inst_id() const { return Inst::Id_Unknown; }

    int _size_in_bits;
};

Expr unknown_inst( int size_in_bits ) {
    Unknown *res = new Unknown;
    res->_size_in_bits = size_in_bits;
    return Expr( res, 0 );
}

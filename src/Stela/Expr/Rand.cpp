#include "InstVisitor.h"
#include "Inst_.h"
#include "Rand.h"

namespace Expr_NS {

/**
*/
class Rand : public Inst_<1,0> {
public:
    virtual int size_in_bits( int nout ) const { return size; }
    virtual void write_to_stream( Stream &os ) const { os << "rand_" << size; }
    virtual void apply( InstVisitor &visitor ) const { visitor( *this, size ); }
    virtual int inst_id() const { return 4; }

    int size;
};

Expr rand( int size_in_bits ) {
    Rand *res = new Rand;
    res->size = size_in_bits;
    return Expr( res, 0 );
}

}

#include "InstVisitor.h"
#include "Inst_.h"
#include "Rand.h"

/**
*/
class Rand : public Inst_<1,0> {
public:
    virtual int size_in_bits( int nout ) const { return size; }
    virtual void write_dot( Stream &os ) const { os << "rand_" << size; }
    virtual void apply( InstVisitor &visitor ) const { visitor.rand( *this, size ); }
    virtual int inst_id() const { return Inst::Id_Rand; }

    int size;
};

Expr rand( int size_in_bits ) {
    Rand *res = new Rand;
    res->size = size_in_bits;
    return Expr( res, 0 );
}


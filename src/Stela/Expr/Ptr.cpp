#include "InstVisitor.h"
#include "Inst_.h"
#include "Ptr.h"

namespace Expr_NS {

/**
*/
class Ptr : public Inst_<1,1> {
public:
    virtual int size_in_bits( int nout ) const { return ptr_size; }
    virtual void write_to_stream( Stream &os ) const { os << "ptr"; }
    virtual void apply( InstVisitor &visitor ) const { visitor( *this, ptr_size ); }
    virtual int inst_id() const { return 3; }
    int ptr_size;
};


Expr ptr( Expr expr, int ptr_size ) {
    Ptr *res = new Ptr;
    res->ptr_size = ptr_size;
    res->inp_repl( 0, expr );
    return Expr( Inst::factorized( res ), 0 );
}

}

#include "InstVisitor.h"
#include "Inst_.h"
#include "Conv.h"
#include "Cst.h"

class Conv : public Inst_<1,1> {
public:
    virtual int size_in_bits( int nout ) const { return dst->size_in_bits(); }
    virtual void write_dot( Stream &os ) const { os << "conv[" << *dst << "," << *src << "]"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.conv( *this, dst, src ); }
    virtual int inst_id() const { return Inst::Id_Conv; }
    virtual bool equal( const Inst *b ) const {
        return Inst::equal( b ) and
               dst == static_cast<const Conv *>( b )->dst and
               src == static_cast<const Conv *>( b )->src;
    }
    virtual const BaseType *out_bt( int n ) const {
        return dst;
    }

    const BaseType *dst;
    const BaseType *src;
};

Expr conv( const BaseType *dst, const BaseType *src, Expr a ) {
    // known value ?
    if ( const PI8 *da = a.cst_data() ) {
        PI8 tmp[ dst->size_in_bytes() ];
        if ( dst->conv( tmp, src, da ) )
            return cst( tmp, 0, dst->size_in_bits() );
    }

    // else, create a new inst
    Conv *res = new Conv;
    res->inp_repl( 0, a );
    res->dst = dst;
    res->src = src;
    return Expr( Inst::factorized( res ), 0 );
}

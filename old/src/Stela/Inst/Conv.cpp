#include "InstVisitor.h"
#include "Inst_.h"
#include "Conv.h"
#include "Cst.h"
#include "Phi.h"

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
    virtual Expr clone( Expr *ch, int nout ) const {
        return conv( dst, src, ch[ 0 ] );
    }
    virtual int sizeof_additionnal_data() const {
        return 2 * sizeof( const BaseType * );
    }
    virtual void copy_additionnal_data_to( PI8 *dsp ) const {
        memcpy( dsp, &dst, 2 * sizeof( const BaseType * ) );
    }
    virtual const BaseType *out_bt( int n ) const {
        return dst;
    }

    const BaseType *dst;
    const BaseType *src;
};

Expr conv( const BaseType *dst, const BaseType *src, Expr a ) {
    // same type ?
    if ( dst == src )
        return a;

    // known value ?
    if ( const PI8 *da = a.cst_data() ) {
        PI8 tmp[ dst->size_in_bytes() ];
        if ( dst->conv( tmp, src, da ) )
            return cst( tmp, 0, dst->size_in_bits() );
    }

    // bool( phi( ... ) ) -> c and b
    if ( dst == bt_Bool and a.inst->inst_id() == Inst::Id_Phi ) {
        // bool( phi( c, conv to true, conv to true ) ) -> true
        Expr vok = conv( dst, src, a.inst->inp_expr( 1 ) );
        Expr vko = conv( dst, src, a.inst->inp_expr( 2 ) );
        if ( vok == vko )
            return vok;

        // bool( phi( bool( c ), b, c ) ) -> phi( bool( c ), bool( b ), false )
        if ( a.inst->inp_expr( 0 ).inst->inst_id() == Inst::Id_Conv and a.inst->inp_expr( 0 ).inst->inp_expr( 0 ) == a.inst->inp_expr( 2 ) ) {
            Expr bool_b = conv( dst, src, a.inst->inp_expr( 1 ) );
            return phi( a.inst->inp_expr( 0 ), bool_b, cst( false ) );
        }

        // bool( phi( bool( c ), c, b ) ) -> phi( bool( c ), true, bool( b ) )
        if ( a.inst->inp_expr( 0 ).inst->inst_id() == Inst::Id_Conv and a.inst->inp_expr( 0 ).inst->inp_expr( 0 ) == a.inst->inp_expr( 1 ) ) {
            Expr bool_b = conv( dst, src, a.inst->inp_expr( 2 ) );
            return phi( a.inst->inp_expr( 0 ), cst( true ), bool_b );
        }
    }

    // else, create a new inst
    Conv *res = new Conv;
    res->inp_repl( 0, a );
    res->dst = dst;
    res->src = src;
    return Expr( Inst::factorized( res ), 0 );
}

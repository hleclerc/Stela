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

    // bool( phi( bool( c ), b, c ) ) -> c and b
    if ( dst == bt_Bool and a.inst->inst_id() == Inst::Id_Phi ) {
        if ( a.inst->inp_expr( 0 ) == a.inst->inp_expr( 2 ) or ( a.inst->inp_expr( 0 ).inst->inst_id() == Inst::Id_Conv and a.inst->inp_expr( 0 ).inst->inp_expr( 0 ) == a.inst->inp_expr( 2 ) ) ) {
            Expr cb = conv( dst, src, a.inst->inp_expr( 1 ) );
            Bool rb;
            if ( cb.get_val( rb )  ) {
                if ( rb )
                    return conv( dst, src, a.inst->inp_expr( 2 ) );
                return cst( false );
            }
        }
    }

    // else, create a new inst
    Conv *res = new Conv;
    res->inp_repl( 0, a );
    res->dst = dst;
    res->src = src;
    return Expr( Inst::factorized( res ), 0 );
}

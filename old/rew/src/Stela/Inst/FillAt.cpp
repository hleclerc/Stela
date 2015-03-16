#include "InstInfo_C.h"
#include "FillAt.h"
#include "Slice.h"
#include "Type.h"
#include "Cst.h"
#include "Op.h"
#include "Ip.h"


/**
  inp[ 0 ] -> src
  inp[ 1 ] -> val
  inp[ 2 ] -> off
*/
class FillAt : public Inst {
public:
    FillAt( Type *off_type ) : off_type( off_type ) {}
    virtual void write_dot( Stream &os ) const { os << "fill_at"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new FillAt( off_type ); }
    virtual int size() const { return inp[ 0 ]->size(); }
    virtual Expr _simp_slice( int off, int len ) {
        SI32 beg;
        if ( inp[ 2 ]->get_val( beg, off_type ) ) {
            SI32 end = beg + inp[ 1 ]->size();
            if ( off + len <= beg or off >= end )
                return slice( inp[ 0 ], off, len );
            if ( off >= beg and off + len <= end )
                return slice( inp[ 1 ], off - beg, len );
        }
        return Inst::_simp_slice( off, len );
    }
    virtual void write_to( Codegen_C *cc, int prec = -1 ) {
        if ( par.size() > 1 )
            TODO;
        cc->add_type( &ip->type_PI8 );
        cc->on << "*(" << *IIC( inp[ 1 ] )->out_type << " *)( (PI8 *)&" <<  cc->code( inp[ 0 ] ) << " + " << cc->code( inp[ 2 ], PREC_div ) << " / 8 ) = " << cc->code( inp[ 1 ] ) << ";";
        IIC( this )->out_reg = IIC( inp[ 0 ] )->out_reg;
    }
    //    virtual void write_to( Codegen_C *cc, int prec, OutReg *out_reg ) {
    //        *cc->os << "2";
    //    }

    Type *off_type;
};

Expr fill_at( Expr src, Expr val, Type *off_type, Expr off ) {
    SI32 off_val;
    if ( off->get_val( off_val, off_type ) ) {
        // complete fill ?
        if ( off_val == 0 and src->size() == val->size() )
            return val;

        //
        PI8 data[ src->sb() ];
        int pb = off_val + val->size();
        if ( src->get_val( data, off_val ) and
             val->get_val( data, val->size(), 0, off_val ) and
             src->get_val( data, src->size() - pb, pb, pb ) ) {
            // TODO: get_val with unknown
            return cst( src->size(), data );
        }
    }



    FillAt *res = new FillAt( off_type );
    res->add_inp( src );
    res->add_inp( val );
    res->add_inp( off );
    return res;
}

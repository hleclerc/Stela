#include "../System/Memcpy.h"
#include "InstVisitor.h"
#include "Inst_.h"
#include "Cst.h"

///
static Vec<class Cst *> cst_set;

///
class Cst : public Inst_<1,0> {
public:
    Cst( const Vec<PI8> &value, const Vec<PI8> &known ) : value( value ), known( known ) {
        cst_set << this;
    }

    virtual ~Cst() {
        cst_set.remove_first_unordered( this );
    }

    virtual int size_in_bits( int nout ) const {
        return value.size() * 8;
    }

    virtual const PI8 *cst_data( int nout ) const {
        return value.ptr();
    }

    virtual void write_to_stream( Stream &os ) const {
        if ( value.size() == 4 )
            os << *reinterpret_cast<const SI32 *>( value.ptr() );
        else if ( value.size() == 8 )
            os << *reinterpret_cast<const SI64 *>( value.ptr() );
        else {
            const char *c = "0123456789ABCDEF";
            for( int i = 0; i < std::min( value.size(), ST( 4 ) ); ++i ) {
                if ( i )
                    os << ' ';
                os << c[ value[ i ] >> 4 ] << c[ value[ i ] & 0xF ];
            }
            if ( value.size() > 4 )
                os << "...";
        }
    }

    virtual void apply( InstVisitor &visitor ) const {
        visitor.cst( *this, value );
    }

    virtual int inst_id() const { return Inst::Id_Cst; }

    virtual Expr _smp_slice( int nout, int beg, int end ) {
        Vec<PI8> data( Size(), ( end - beg + 7 ) / 8 );
        memcpy_bit( data.ptr(), 0, value.ptr(), beg, end - beg );
        return cst( data );
    }

    Vec<PI8> value; ///< value (should not be changed directly)
    Vec<PI8> known; ///< known bits
};

Expr cst( const Vec<PI8> &value, const Vec<PI8> &known ) {
    // already an equivalent cst ?
    for( int i = 0; i < cst_set.size(); ++i )
        if ( cst_set[ i ]->value == value and cst_set[ i ]->known == known )
            return Expr( cst_set[ i ], 0 );

    // else, create a new one
    return Expr( new Cst( value, known ), 0 );
}

Expr cst( const Vec<PI8> &value ) {
    return cst( value, Vec<PI8>( Size(), value.size(), 0xFF ) );
}

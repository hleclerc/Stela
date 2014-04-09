#include "../System/Memcpy.h"
#include "InstVisitor.h"
#include "Inst_.h"
#include "Cst.h"

///
static Vec<class Cst *> cst_set;

///
class Cst : public Inst_<1,0> {
public:
    Cst( const PI8 *value, const PI8 *known, int size_in_bits ) : data( Size(), 2 * ( ( size_in_bits + 7 ) / 8 ) ), size( size_in_bits ) {
        int sb = ( size_in_bits + 7 ) / 8;
        memcpy_bit( data.ptr() + 0 * sb, 0, value, 0, size_in_bits );
        if ( known )
            memcpy_bit( data.ptr() + 1 * sb, 0, known, 0, size_in_bits );
        else
            for( int i = 1 * sb; i < 2 * sb; ++i )
                data[ i ] = 0xFF;
        cst_set << this;
    }
    virtual ~Cst() {
        cst_set.remove_first_unordered( this );
    }
    virtual int size_in_bits( int nout ) const {
        return size;
    }
    virtual const PI8 *cst_data( int nout, int beg, int end ) const {
        if ( beg % 8 )
            TODO;
        return data.ptr() + beg / 8;
    }
    virtual Expr _smp_slice( int nout, int beg, int end ) {
        if ( beg == 0 and end == size )
            return Expr( this, nout );
        if ( beg == end )
            return cst();
        if ( beg % 8 == 0 and end % 8 == 0 )
            return cst( data.ptr() + beg / 8, data.ptr() + ( size + 7 ) / 8 + beg / 8, end - beg );
        return Expr();
    }
    virtual void write_to_stream( Stream &os ) const {
        write_dot( os );
    }
    virtual void write_dot( Stream &os ) const {
        if ( size == 0 )
            os << "";
        else if ( size <= 8 )
            os << (int)*reinterpret_cast<const PI8 *>( data.ptr() );
        else if ( size == 16 )
            os << *reinterpret_cast<const SI16 *>( data.ptr() );
        else if ( size == 32 )
            os << *reinterpret_cast<const SI32 *>( data.ptr() );
        else if ( size == 64 )
            os << *reinterpret_cast<const SI64 *>( data.ptr() );
        else {
            const char *c = "0123456789ABCDEF";
            for( int i = 0; i < std::min( size / 8, 4 ); ++i ) {
                if ( i )
                    os << ' ';
                os << c[ data[ i ] >> 4 ] << c[ data[ i ] & 0xF ];
            }
            if ( size / 8 > 4 )
                os << "...";
        }
    }
    virtual int sizeof_additionnal_data() const {
        return sizeof( int ) + ( size + 7 ) / 8 * 2;
    }
    virtual void copy_additionnal_data_to( PI8 *dst ) const {
        memcpy( dst, &size, sizeof( int ) );
        memcpy( dst + sizeof( int ), data.ptr(), ( size + 7 ) / 8 * 2 );
    }
    virtual void apply( InstVisitor &visitor ) const {
        int sb = ( size + 7 ) / 8;
        visitor.cst( *this, data.ptr() + 0 * sb, data.ptr() + 1 * sb, size );
    }
    virtual int inst_id() const {
        return Inst::Id_Cst;
    }

    Vec<PI8> data; ///< values and known (should not be changed directly)
    Vec<PI8> shifted_data;
    int size;
};

static bool equal_cst( const Cst *cst, const PI8 *ptr, const PI8 *kno, int size_in_bits ) {
    if ( cst->size != size_in_bits )
        return false;
    int sb = ( size_in_bits + 7 ) / 8;
    for( int i = 0; i < sb; ++i )
        if ( ( cst->data[ i ] & cst->data[ i + sb ] ) != ( ptr[ i ] & cst->data[ i + sb ] ) )
            return false;
    if ( kno ) {
        for( int i = 0; i < sb; ++i )
            if ( cst->data[ i + sb ] != kno[ i ] )
                return false;
    } else {
        for( int i = 0; i < sb; ++i )
            if ( cst->data[ i + sb ] != 0xFF )
                return false;
    }
    return true;
}

Expr cst( const PI8 *ptr, const PI8 *kno, int size_in_bits ) {
    if ( size_in_bits and not ptr ) {
        ASSERT( kno == 0, "weird" );
        int sb = ( size_in_bits + 7 ) / 8;
        PI8 nptr[ sb ];
        PI8 nkno[ sb ];
        for( int i = 0; i < sb; ++i ) {
            nptr[ i ] = 0x00;
            nkno[ i ] = 0x00;
        }
        return cst( nptr, nkno, size_in_bits );
    }

    // already an equivalent cst ?
    for( int i = 0; i < cst_set.size(); ++i )
        if ( equal_cst( cst_set[ i ], ptr, kno, size_in_bits ) )
            return Expr( cst_set[ i ], 0 );

    // else, create a new one
    return Expr( new Cst( ptr, kno, size_in_bits ), 0 );
}

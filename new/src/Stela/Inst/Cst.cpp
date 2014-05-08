#include "../System/Memcpy.h"
#include "Cst.h"

static Vec<class Cst *> cst_set;

/**
*/
class Cst : public Inst {
public:
    Cst( int size_in_bits, const PI8 *value, const PI8 *known ) : data( Size(), 2 * ( ( size_in_bits + 7 ) / 8 ) ), size( size_in_bits ) {
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
    Vec<PI8> data; ///< data and knwn
    int size;
};

static bool equal_cst( const Cst *cst, int size_in_bits, const PI8 *ptr, const PI8 *kno ) {
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

Ptr<Inst> cst( int size, const void *data, const void *knwn ) {
    PRINT( size );
    if ( size and not data ) {
        ASSERT( knwn == 0, "weird" );
        // -> make a knwn full of 0
        int sb = ( size + 7 ) / 8;
        PI8 nptr[ sb ];
        PI8 nkno[ sb ];
        for( int i = 0; i < sb; ++i ) {
            nptr[ i ] = 0x00;
            nkno[ i ] = 0x00;
        }
        return cst( size, nptr, nkno );
    }

    // already an equivalent cst ?
    for( int i = 0; i < cst_set.size(); ++i )
        if ( equal_cst( cst_set[ i ], size, (const PI8 *)data, (const PI8 *)knwn ) )
            return cst_set[ i ];

    // else, create a new one
    return new Cst( size, (const PI8 *)data, (const PI8 *)knwn );
}

#include "../CodeGen/CodeGen_C.h"
#include "../System/Memcpy.h"
#include "Cst.h"
#include "Ip.h"

static Vec<class Cst *> cst_set;

/**
*/
class Cst : public Inst {
public:
    Cst( int size_in_bits, const PI8 *value, const PI8 *known ) : data( Size(), 2 * ( ( size_in_bits + 7 ) / 8 ) ), len( size_in_bits ) {
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
        if ( len == 0 )
            os << "";
        else if ( len <= 8 )
            os << (int)*reinterpret_cast<const PI8 *>( data.ptr() );
        else if ( len == 16 )
            os << *reinterpret_cast<const SI16 *>( data.ptr() );
        else if ( len == 32 )
            os << *reinterpret_cast<const SI32 *>( data.ptr() );
        else if ( len == 64 )
            os << *reinterpret_cast<const SI64 *>( data.ptr() );
        else {
            const char *c = "0123456789ABCDEF";
            for( int i = 0; i < std::min( len / 8, 4 ); ++i ) {
                if ( i )
                    os << ' ';
                os << c[ data[ i ] >> 4 ] << c[ data[ i ] & 0xF ];
            }
            if ( len / 8 > 4 )
                os << "...";
        }
    }
    virtual int size() const {
        return len;
    }
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const {
        return new Cst( len, data.ptr(), data.ptr() + ( len + 7 ) / 8 );
    }
    virtual void write_1l_to( CodeGen_C *cc ) const {
        Type *type = out_type_proposition( cc );
        if ( all_known() ) {
            if ( type == &ip->type_SI32 ) { *cc->os << *reinterpret_cast<const SI32 *>( data.ptr() ); return; }
            if ( type == &ip->type_SI64 ) { *cc->os << *reinterpret_cast<const SI64 *>( data.ptr() ) << "ll"; return; }
            if ( type == &ip->type_Bool ) { *cc->os << ( *reinterpret_cast<const Bool *>( data.ptr() ) & 1 ? "true" : "false" ); return; }
        }
        *cc->os << *type;
    }
    virtual const PI8 *data_ptr( int offset ) const {
        if ( offset % 8 ) {
            IP_ERROR( "todo" );
            return 0;
        }
        return data.ptr() + offset / 8;
    }
    bool all_known() const {
        for( int sb = ( len + 7 ) / 8, i = 1 * sb; i < 2 * sb; ++i )
            if ( data[ i ] != 0xFF )
                return false;
        return true;
    }

    Vec<PI8> data; ///< data and knwn
    int len; ///< in bits
};

static bool equal_cst( const Cst *cst, int size_in_bits, const PI8 *ptr, const PI8 *kno ) {
    if ( cst->len != size_in_bits )
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

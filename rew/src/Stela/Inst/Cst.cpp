#include "../System/Memcpy.h"
#include "InstInfo_C.h"
#include "BoolOpSeq.h"
#include "Type.h"
#include "Cst.h"
#include "Ip.h"

class Cst;
static Vec<Cst *> cst_list;

/**
*/
class Cst : public Inst {
public:
    Cst( int len, const void *data, const void *kwnw ) : len( len ) {
        this->data.resize( 2 * sb() );
        memcpy( d(), data, sb() );
        memcpy( k(), kwnw, sb() );

        ++cpt_use;
        cst_list << this;
    }
    int sb() const { return ( len + 7 ) / 8; }
    const PI8 *d() const { return data.ptr() + 0 * sb(); }
    const PI8 *k() const { return data.ptr() + 1 * sb(); }
    PI8 *d() { return data.ptr() + 0 * sb(); }
    PI8 *k() { return data.ptr() + 1 * sb(); }

    virtual void write_dot( Stream &os ) const {
        if ( len == 0 )
            os << "";
        else if ( len <= 8 )
            os << (int)*reinterpret_cast<const PI8 *>( d() );
        else if ( len == 16 )
            os << *reinterpret_cast<const SI16 *>( d() );
        else if ( len == 32 )
            os << *reinterpret_cast<const SI32 *>( d() );
        else if ( len == 64 )
            os << *reinterpret_cast<const SI64 *>( d() );
        else {
            const char *c = "0123456789ABCDEF";
            for( int i = 0; i < std::min( sb(), 4 ); ++i ) {
                if ( i )
                    os << ' ';
                os << c[ data[ i ] >> 4 ] << c[ data[ i ] & 0xF ];
            }
            if ( sb() > 4 )
                os << "...";
        }

    }

    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Cst( len, d(), k() );
    }

    virtual int size() const {
        return len;
    }

    bool get_val( void *dst, int size, int offset = 0, int dst_offset = 0 ) const {
        if ( offset + size > len )
            return false;
        memcpy_bit( dst, dst_offset, d(), offset, size );
        return true;
    }
    virtual Expr _simp_slice( int off, int len ) {
        int s = ( len + 7 ) / 8;
        PI8 data[ s ];
        PI8 knwn[ s ];
        memcpy_bit( data, 0, d(), off, len );
        memcpy_bit( knwn, 0, k(), off, len );
        return cst( len, data, knwn );
    }

    bool equal( int l, const PI8 *nd, const PI8 *nk ) const {
        if ( l != len )
            return false;
        for( int i = 0; i < sb(); ++i )
            if ( nd[ i ] != d()[ i ] )
                return false;
        for( int i = 0; i < sb(); ++i )
            if ( nk[ i ] != k()[ i ] )
                return false;
        return true;
    }
    virtual int always_checked() const {
        if ( k()[ 0 ] & 1 )
            return d()[ 0 ] & 1 ? 1 : -1;
        return 0;
    }
    virtual bool going_to_write_c_code() {
        return false;
    }
    virtual void write_to( Codegen_C *cc, int prec ) {
        if ( prec < 0 )
            return;
        if ( IIC( this )->out_type == &ip->type_Bool ) *cc->os << ( *d() ? "true" : "false" );
        else if ( IIC( this )->out_type == &ip->type_SI8  ) *cc->os << "SI8( "  << int( *reinterpret_cast<const SI8  *>( d() ) ) << " )";
        else if ( IIC( this )->out_type == &ip->type_PI8  ) *cc->os << "PI8( "  << int( *reinterpret_cast<const PI8  *>( d() ) ) << " )";
        else if ( IIC( this )->out_type == &ip->type_SI16 ) *cc->os << "SI16( " << int( *reinterpret_cast<const SI16 *>( d() ) ) << " )";
        else if ( IIC( this )->out_type == &ip->type_PI16 ) *cc->os << "PI16( " << int( *reinterpret_cast<const PI16 *>( d() ) ) << " )";
        else if ( IIC( this )->out_type == &ip->type_SI32 ) *cc->os << *reinterpret_cast<const SI32 *>( d() );
        else if ( IIC( this )->out_type == &ip->type_PI32 ) *cc->os << *reinterpret_cast<const PI32 *>( d() ) << "u";
        else if ( IIC( this )->out_type == &ip->type_SI64 ) *cc->os << *reinterpret_cast<const SI64 *>( d() ) << "l";
        else if ( IIC( this )->out_type == &ip->type_PI64 ) *cc->os << *reinterpret_cast<const PI64 *>( d() ) << "lu";
        else if ( IIC( this )->out_type == &ip->type_FP32 ) *cc->os << std::scientific << *reinterpret_cast<const FP32 *>( d() );
        else if ( IIC( this )->out_type == &ip->type_FP64 ) *cc->os << std::scientific << *reinterpret_cast<const FP64 *>( d() );
        else {
            *cc->os << *IIC( this )->out_type << "{ ";
            for( int i = 0; i < sb(); ++i ) {
                if ( i )
                    *cc->os << ", ";
                *cc->os << int( d()[ i ] );
            }
            *cc->os << " }";
        }
    }
    virtual BoolOpSeq get_BoolOpSeq() {
        return BoolOpSeq( len ? data[ 0 ] : false );
    }



    int len;
    Vec<PI8> data;
};

Expr cst( int len, const void *data, const void *kwnw ) {
    int sb = ( len + 7 ) / 8;
    if ( not data ) {
        PI8 nd[ sb ];
        for( int i = 0; i < sb; ++i )
            nd[ i ] = 0x00;
        return cst( len, nd, nd );
    }
    if ( not kwnw ) {
        PI8 nk[ sb ];
        for( int i = 0; i < sb; ++i )
            nk[ i ] = 0xFF;
        return cst( len, data, nk );
    }

    // normalize data
    PI8 nd[ sb ];
    for( int i = 0; i < sb; ++i )
        nd[ i ] = reinterpret_cast<const PI8 *>( data )[ i ] & reinterpret_cast<const PI8 *>( kwnw )[ i ];

    // already created ?
    for( Cst *c : cst_list )
        if ( c->equal( len, nd, (PI8 *)kwnw ) )
            return c;
    // else, create a new one
    return new Cst( len, nd, kwnw );
}

Expr cst( bool val ) {
    return cst( 1, &val );
}

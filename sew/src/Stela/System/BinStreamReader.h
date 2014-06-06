#ifndef BINSTREAMREADER_H
#define BINSTREAMREADER_H

#include "TypeConfig.h"
#include <string.h>
#include <endian.h>

#include "Stream.h"

/**
*/
class BinStreamReader {
public:
    struct FactoryRead_PI {
        FactoryRead_PI( BinStreamReader *b ) : b( b ) {}
        template<class T>
        operator T() {
            T res;
            *b >> res;
            return res;
        }
        BinStreamReader *b;
    };

    struct FactoryRead_LE {
        FactoryRead_LE( BinStreamReader *b ) : b( b ) {}
        template<class T>
        operator T() {
            T res;
            b->read( &res, sizeof( res ) );
            #if __BYTE_ORDER == __LITTLE_ENDIAN
            // -> do nothing
            #elif __BYTE_ORDER == __BIG_ENDIAN
            for( int i = 0, j = sizeof( T ) - 1; i < j; ++i, --j )
                std::swap( reinterpret_cast<PI8 *>( &res )[ i ], reinterpret_cast<PI8 *>( &res )[ j ] );
            #else
            #error unknown byte order
            #endif
            return res;
        }
        BinStreamReader *b;
    };

    BinStreamReader( const PI8 *ptr = 0 ) : ptr( ptr ) {}

    operator bool() const { return ptr; }

    BinStreamReader &operator>>( PI8 &val ) {
        val = *( ptr++ );
        return *this;
    }

    template<class T>
    BinStreamReader &operator>>( T &val ) {
        val = 0;
        for( T mul = 1; ; mul *= 128 ) {
            PI8 tmp = *( ptr++ );
            if ( tmp < 128 ) {
                val += mul * tmp;
                break;
            }
            val += mul * ( tmp - 128 );
        }
        return *this;
    }

    FactoryRead_PI read_positive_integer() {
        return this;
    }

    FactoryRead_LE read_le() { ///< little endian data reader
        return this;
    }

    const char *read_cstr() {
        const char *res = reinterpret_cast<const char *>( ptr );
        while ( *( ptr++ ) );
        return res;
    }

    const PI8 *read_offset() {
        const PI8 *old = ptr;
        int off = read_positive_integer();
        return off ? old + off : 0;
    }

    void read( void *res, int len ) {
        memcpy( res, ptr, len );
        ptr += len;
    }

    template<class T>
    T get() {
        return *( ptr++ );
    }

    const PI8 *ptr;
};

#endif // BINSTREAMREADER_H

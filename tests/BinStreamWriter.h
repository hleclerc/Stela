#ifndef BINSTREAMWRITER_H
#define BINSTREAMWRITER_H

#include <string.h>

/**
*/
class BinStreamWriter {
public:
    BinStreamWriter( void *ptr ) : ptr( reinterpret_cast<unsigned char *>( ptr ) ) {}

    void write( const void *data, int size ) {
        memcpy( ptr, data, size );
        ptr += size;
    }

    template<class T>
    BinStreamWriter &write_unsigned( T val ) {
        for( ; val >= 128; val /= 128 )
            *(ptr++) = 128 + ( val % 128 );
        *(ptr++) = val;
        return *this;
    }

    BinStreamWriter &write_unsigned( unsigned char val ) {
        *(ptr++) = val;
        return *this;
    }

    template<class T>
    BinStreamWriter &write_signed( T val ) {
        // negative ?
        if ( val < 0 ) {
            val = - val;
            if ( val >= 64 ) {
                *(ptr++) = 128 + 64 + ( val % 64 );
                for( val /= 64 ; val >= 128; val /= 128 )
                    *(ptr++) = 128 + ( val % 128 );
                *(ptr++) = val;
            } else
                *(ptr++) = 128 + val;
            return *this;
        }
        // -> positive
        if ( val >= 64 ) {
            *(ptr++) = 64 + ( val % 64 );
            for( val /= 64 ; val >= 128; val /= 128 )
                *(ptr++) = 128 + ( val % 128 );
        }
        *(ptr++) = val;
        return *this;
    }

    BinStreamWriter &write_unsigned( signed char val ) {
        *(ptr++) = val;
        return *this;
    }

    unsigned char *ptr;
};

#endif // BINSTREAMWRITER_H

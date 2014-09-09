#ifndef BINSTREAMREADER_H
#define BINSTREAMREADER_H

#ifndef PRINT
    #include <iostream>
    #define PRINT( A ) \
        std::cout << "  " << #A << " -> " << (A) << std::endl
#endif // PRINT

#ifdef METIL_COMP_DIRECTIVE
    #pragma cpp_flag -std=gnu++11
#endif // METIL_COMP_DIRECTIVE

/**
*/
class BinStreamReader {
public:
    struct UnsignedReader {
        UnsignedReader( BinStreamReader *b ) : b( b ) {
        }
        template<class T>
        operator T() {
            if ( b->beg >= b->end )
                return 0;
            T res = *( b->beg++ );
            if ( res >= 128 ) {
                res -= 128;
                int shift = 7;
                while ( true ) {
                    if ( b->beg >= b->end )
                        return res;
                    unsigned char v = *( b->beg++ );
                    if ( v < 128 )
                        return res + ( v << shift );
                    res += ( v - 128 ) << shift;
                    shift += 7;
                }
            }
            return res;
        }
        BinStreamReader *b;
    };

    struct SignedReader {
        SignedReader( BinStreamReader *b ) : b( b ) {
        }
        template<class T>
        operator T() {
            if ( b->beg >= b->end )
                return 0;
            T res = *( b->beg++ );

            // negative ?
            if ( res >= 128 ) {
                res -= 128;
                if ( res >= 64 ) {
                    res = 64 - res;
                    int shift = 6;
                    while ( true ) {
                        if ( b->beg >= b->end )
                            return res;
                        unsigned char v = *( b->beg++ );
                        if ( v < 128 )
                            return res - ( v << shift );
                        res -= ( v - 128 ) << shift;
                        shift += 7;
                    }
                    return res;
                }
                return - res;
            }

            // -> positive
            if ( res >= 64 ) {
                res -= 64;
                int shift = 6;
                while ( true ) {
                    if ( b->beg >= b->end )
                        return res;
                    unsigned char v = *( b->beg++ );
                    if ( v < 128 )
                        return res + ( v << shift );
                    res += ( v - 128 ) << shift;
                    shift += 7;
                }
            }
            return res;
        }
        BinStreamReader *b;
    };


    BinStreamReader( void *beg, void *end ) :
        beg( reinterpret_cast<unsigned char *>( beg ) ),
        end( reinterpret_cast<unsigned char *>( end ) ) {
    }

    UnsignedReader read_unsigned() {
        return this;
    }

    SignedReader read_signed() {
        return this;
    }

protected:
    unsigned char *beg;
    unsigned char *end;
};

#endif // BINSTREAMREADER_H

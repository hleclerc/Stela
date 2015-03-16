#ifndef BUFSTREAMREADER_H
#define BUFSTREAMREADER_H

#include <Celo/System/Buffer.h>
#include <iostream>
#define PRINT( A ) \
    std::cout << "  " << #A << " -> " << (A) << std::endl
#ifdef METIL_COMP_DIRECTIVE
#pragma cpp_flag -std=gnu++11
#endif // METIL_COMP_DIRECTIVE

/**
*/
class BufStreamReader {
public:
    struct UnsignedReader {
        UnsignedReader( BufStreamReader *b ) : b( b ) {
        }
        template<class T>
        operator T() {
            if ( not b->update_beg() )
                return 0;
            T res = b->beg->data[ b->off++ ];
            if ( res >= 128 ) {
                res -= 128;
                int shift = 7;
                while ( true ) {
                    if ( not b->update_beg() )
                        return res;
                    unsigned char v = b->beg->data[ b->off++ ];
                    if ( v < 128 )
                        return res + ( v << shift );
                    res += ( v - 128 ) << shift;
                    shift += 7;
                }
            }
            return res;
        }
        BufStreamReader *b;
    };

    struct SignedReader {
        SignedReader( BufStreamReader *b ) : b( b ) {
        }
        template<class T>
        operator T() {
            if ( not b->update_beg() )
                return 0;
            T res = b->beg->data[ b->off++ ];

            // negative ?
            if ( res >= 128 ) {
                res -= 128;
                if ( res >= 64 ) {
                    res = 64 - res;
                    int shift = 6;
                    while ( true ) {
                        if ( not b->update_beg() )
                            return res;
                        unsigned char v = b->beg->data[ b->off++ ];
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
                    if ( not b->update_beg() )
                        return res;
                    unsigned char v = b->beg->data[ b->off++ ];
                    if ( v < 128 )
                        return res + ( v << shift );
                    res += ( v - 128 ) << shift;
                    shift += 7;
                }
            }
            return res;
        }
        BufStreamReader *b;
    };


    BufStreamReader( Celo::Ptr<Celo::Buffer> beg, int off );

    UnsignedReader read_unsigned() {
        return UnsignedReader{ this };
    }

    SignedReader read_signed() {
        return SignedReader{ this };
    }

protected:
    bool update_beg() {
        while ( off >= beg->used ) {
            off -= beg->used;
            beg = beg->next;
            if ( not beg )
                return false;
        }
        return true;
    }


    Celo::Ptr<Celo::Buffer> beg;
    int                     off; ///<
};

#endif // BUFSTREAMREADER_H

#ifndef BINSTREAMWRITER_H
#define BINSTREAMWRITER_H

#include "SplittedVec.h"
#include "Assert.h"
#include "Stream.h"
#include <string.h>

/**
  Helper to construct binary streams of instructions
*/
class BinStreamWriter {
public:
    struct AutoOffset {
        AutoOffset( BinStreamWriter &toker );
        ~AutoOffset();
        BinStreamWriter *toker;
        ST old_size;
        int *ptr;
    };

    struct RoomForFactory {
        RoomForFactory( BinStreamWriter *b ) : b( b ) {}
        template<class T>
        operator T*() { return reinterpret_cast<T *>( b->data.get_room_for( sizeof( T ) ) ); }
        BinStreamWriter *b;
    };

    BinStreamWriter();
    void write_to_stream( Stream &os ) const; ///< display data

    // get the resulting data
    void copy_to( PI8 *ptr ) const { data.copy_to( ptr ); }
    ST size() const { return data.size(); }


    // write instructions
    void write_le( SI16 val ) { *(SI16 *)room_for() = htole16( val ); } ///< write little endian
    void write_le( PI16 val ) { *(PI16 *)room_for() = htole16( val ); } ///< write little endian

    void write_le( SI32 val ) { *(SI32 *)room_for() = htole32( val ); } ///< write little endian
    void write_le( PI32 val ) { *(PI32 *)room_for() = htole32( val ); } ///< write little endian

    void write_le( SI64 val ) { *(SI64 *)room_for() = htole64( val ); } ///< write little endian
    void write_le( PI64 val ) { *(PI64 *)room_for() = htole64( val ); } ///< write little endian

    template<class T>
    void write_le( T val ) {
        #if __BYTE_ORDER == __LITTLE_ENDIAN
        // -> do nothing
        #elif __BYTE_ORDER == __BIG_ENDIAN
        for( int i = 0, j = sizeof( T ) - 1; i < j; ++i, --j )
            std::swap( reinterpret_cast<PI8 *>( &val )[ i ], reinterpret_cast<PI8 *>( &val )[ j ] );
        #else
        #error unknown byte order
        #endif
        *(T *)room_for() = val;
    }

    void write( const void *ptr, int len ); ///< append raw binary data


    /// BEWARE : val must be a positive integer
    template<class T>
    BinStreamWriter &operator<<( T val ) {
        ASSERT_IF_DEBUG( val >= 0 ); // , "in binary instruction streams only positive values are stored"
        for( ; val >= 128; val /= 128 )
            data << 128 + ( val % 128 );
        data << val;
        return *this;
    }

    BinStreamWriter &operator<<( PI8 val ) {
        data << val;
        return *this;
    }

    /// BEWARE : val must be positive
    template<class T>
    static ST size_needed_for( T val ) {
        ASSERT_IF_DEBUG( val >= 0 ); // , "in binary instruction streams we only store positive values"
        ST res = 1;
        for( ; val >= 128; val /= 128 )
            ++res;
        return res;
    }

    static ST size_needed_for( PI8 val ) {
        return 1;
    }

    template<class T>
    static void write_val_to( PI8 *&ptr, T val ) {
        ASSERT_IF_DEBUG( val >= 0 ); // , "in binary instruction streams we only store positive values"
        for( ; val >= 128; val /= 128 )
            *(ptr++) = 128 + ( val % 128 );
        *(ptr++) = val;
    }

    static void write_val_to( PI8 *&ptr, PI8 val ) {
        *(ptr++) = val;
    }

    // reservation
    RoomForFactory room_for() { ///< ex: int *a = bin.room_for(); -> will allocate the room for an int
        return this;
    }

    //
    void clear() { data.clear(); }

//protected:
    friend struct AutoOffset;
    SplittedVec<PI8,128> data;
};


#endif // BINSTREAMWRITER_H

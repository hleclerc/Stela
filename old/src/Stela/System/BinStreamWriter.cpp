#include "BinStreamWriter.h"

static void write_little_endian( int *ptr, int res ) {
    #if __BYTE_ORDER != __LITTLE_ENDIAN
    res = __bswap_32( res );
    #endif
    *ptr = res;
}


BinStreamWriter::AutoOffset::AutoOffset( BinStreamWriter &toker ) : toker( &toker ) {
    old_size = toker.data.size();
    ptr = reinterpret_cast<int *>( toker.data.get_room_for( sizeof( int ) ) );
}

BinStreamWriter::AutoOffset::~AutoOffset() {
    write_little_endian( ptr, toker->data.size() - old_size );
}


BinStreamWriter::BinStreamWriter() {
}

void BinStreamWriter::write_to_stream( Stream &os ) const {
    for( int i = 0; i < data.size(); ++i ) {
        static const char *c = "0123456789abcdef";
        os << ( i ? ", " : "" ) << "0x" << c[ data[ i ] / 16 ] << c[ data[ i ] % 16 ];
    }
    os << "\n";
}

void BinStreamWriter::write( const void *ptr, int len ) {
    memcpy( data.get_room_for( len ), ptr, len );
}


#include "Type.h"

Type::Type() {
    _len = -1;
}

void Type::write_to_stream( Stream &os ) {
    os << "type";
}

void Type::write_to_stream( Stream &os, void *data, int len ) {
    int sb = ( len + 7 ) / 8;
    const char *c = "0123456789ABCDEF";
    for( int i = 0; i < std::min( sb, 4 ); ++i )
        os << c[ reinterpret_cast<PI8 *>( data )[ i ] >> 4 ]
           << c[ reinterpret_cast<PI8 *>( data )[ i ] & 0xF ];
    if ( sb > 4 )
        os << "...";
}

int Type::size() {
    if ( _len < 0 )
        parse();
    return _len;
}

void Type::parse() {

}

#include "Class.h"

Type::Type( Class *orig ) : orig( orig ) {
    _len  = -1;
    aryth = false;
}

void Type::write_to_stream( Stream &os ) {
    os << *orig;

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

int Type::sb() {
    int s = size();
    return s >= 0 ? ( s + 7 ) / 8 : -1;
}

void Type::parse() {

}

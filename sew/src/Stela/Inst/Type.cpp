#include "Class.h"
#include "Type.h"
#include "Ip.h"

Type::Type( Class *orig ) : orig( orig ) {
    _len    = -1;
    _pod    = -1;
    aryth   = false;
}

void Type::write_to_stream( Stream &os ) {
    os << *orig;
    if ( parameters.size() ) {
        os << "[" << parameters[ 0 ]->get();
        for( int i = 1; i < parameters.size(); ++i )
            os << "," << parameters[ i ]->get();
        os << "]";
    }
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

Expr Type::size( Inst *inst ) {
    if ( _len >= 0 )
        return _len;
    TODO;
    return 0;
}

int Type::size() {
    return _len;
}

int Type::pod() {
    return _pod;
}

int Type::sb() {
    int s = size();
    return s >= 0 ? ( s + 7 ) / 8 : -1;
}


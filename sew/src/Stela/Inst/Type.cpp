#include "Class.h"
#include "Type.h"
#include "Ip.h"

Type::Type( Class *orig ) : orig( orig ) {
    _len    = -1;
    _pod    = -1;
    _parsed = false;
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
    parse();
    if ( _len >= 0 )
        return _len;
    TODO;
    return 0;
}

int Type::size() {
    if ( _len < 0 )
        parse();
    return _len;
}

int Type::pod() {
    if ( _pod < 0 )
        parse();
    return _pod;
}

int Type::sb() {
    int s = size();
    return s >= 0 ? ( s + 7 ) / 8 : -1;
}

void Type::parse() {
    if ( _parsed )
        return;
    _parsed = true;

    if ( not orig->block.tok )
        return ip->disp_error( "Attempting to parse class '" + ip->str_cor.str( orig->name ) + "' which is not defined." );

    TODO;
}

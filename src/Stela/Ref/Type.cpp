#include "Ip.h"

Type::Type( int name ) : bt( 0 ), name( name ) {
    _size = -1;
}

void Type::write_to_stream( Stream &os ) const {
    os << ip->str_cor.str( name );
}

int Type::size() {
    if ( _size < 0 )
        parse();
    return _size;
}

bool Type::pod() const {
    return true;
}

void Type::parse() {
    TODO;
}

void Type::add_room( int size ) {
    if ( _size < 0 )
        _size = 0;
    _size += size;
}

void Type::add_field( int name, Var data ) {
    TODO;
}

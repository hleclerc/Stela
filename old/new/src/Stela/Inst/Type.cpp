#include "Type.h"
#include "Ip.h"

Type::Type( int name ) : name( name ) {
    _size = -1;
}

void Type::parse() {
}

void Type::write_to_stream( Stream &os ) const {
    os << ip->str_cor.str( name );
}

int Type::size() {
    if( _size < 0 )
        parse();
    return _size;
}

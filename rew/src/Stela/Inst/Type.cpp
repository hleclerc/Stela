#include "Type.h"
#include "Ip.h"

Type::Type( int name, int len ) : name( name ), _len( len ) {
    _pod = true;
}

void Type::write_to_stream( Stream &os ) const {
    os << ip->str_cor.str( name );
}

int Type::pod() {
    return _pod;
}

int Type::size() {
    if ( _len < 0 )
        parse();
    return _len;
}

void Type::parse() {
    PRINT( name );
    TODO;
}

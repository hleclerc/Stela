#include "Type.h"
#include "Ip.h"

Type::Type( int name, int len ) : name( name ), _len( len ) {
    _pod = true;
}

void Type::write_to_stream( Stream &os ) const {
    os << ip->str_cor.str( name );
}

void Type::write_C_decl( Stream &out ) const {
    switch( name ) {
        case STRING_SI32_NUM: out << "typedef int SI32;\n"; break;
        default:
            out << "struct " << *this << " {\n";
            out << "    char data[ " << ( size() + 7 ) / 8 << " ];\n";
            out << "};\n";
    }
}

int Type::pod() const {
    return _pod;
}

int Type::size() const {
    if ( _len < 0 )
        parse();
    return _len;
}

void Type::parse() const {
    PRINT( name );
    TODO;
}

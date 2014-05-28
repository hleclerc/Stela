#include "Type.h"
#include "Ip.h"

Type::Type( int name ) : name( name ), _len( -1 ) {
    orig = 0;
    _pod = true;
}

void Type::write_to_stream( Stream &os ) const {
    os << ip->str_cor.str( name );
    if ( parameters.size() ) {
        os << "[ ";
        for( int i = 0; i < parameters.size(); ++i ) {
            if ( i )
                os << ", ";
            os << parameters[ i ];
        }
        os << " ]";
    }
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
    PRINT( orig->name );
    PRINT( *this );
    TODO;
}

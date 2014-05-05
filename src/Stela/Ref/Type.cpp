#include "Type.h"

Type::Type() {
}

void Type::write_to_stream( Stream &os ) const {
    os << "type";
}

int Type::size_in_bits() const {
    return 0;
}

int Type::size_in_bytes() const {
    return ( size_in_bits() + 7 ) / 8;
}

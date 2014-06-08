#include "Class.h"
#include "Ip.h"

Class::Class() {
}

void Class::write_to_stream( Stream &os ) const {
    os << ip->str_cor.str( name );
}

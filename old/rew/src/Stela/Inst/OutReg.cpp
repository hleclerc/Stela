#include "OutReg.h"

OutReg::OutReg( Type *type, int num ) : type( type ), num( num ) {
}

void OutReg::write_to_stream( Stream &os ) const {
    os << "R" << num;
}

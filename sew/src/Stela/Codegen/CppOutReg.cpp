#include "../Inst/Type.h"
#include "CppOutReg.h"
#include "Codegen_C.h"

CppOutReg::CppOutReg( Type *type, int num ) : type( type ), num( num ) {
}

Stream &CppOutReg::write( Codegen_C *cc, bool new_reg ) {
    if ( new_reg )
        *cc->os << *type << " ";
    return *cc->os << "R" << num;
}

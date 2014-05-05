#include "../Inst/Cst.h"
#include "RefItem_Expr.h"
#include "Var.h"
#include "Ip.h"

Var::Var( Type *type, int size, PI8 *data, PI8 *knwn ) :
    ref( new Ref( type, new RefItem_Expr( cst( data, knwn, size ) ) ) ),
    flags( 0 ) {
}

Var::Var() : flags( 0 ) {
}

Var Var::from_val( int val ) {
    return Var( &ip->type_SI32, 32, (PI8 *)&val );
}


void Var::write_to_stream( Stream &os ) const {
    os << ref;
}

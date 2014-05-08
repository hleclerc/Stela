#include "Type.h"
#include "Room.h"
#include "Var.h"
#include "Cst.h"
#include "Ip.h"

Var::Var( Type *type ) : type( type ), flags( 0 ) {
    if ( type ) {
        inst = room( type );
        inst->add_var_ptr( this );
    }
}

Var::Var( SI32 val ) : Var( &ip->type_SI32 ) {
    inst->set( cst( val ) );
}

void Var::write_to_stream( Stream &os ) const {
    os << inst;
}

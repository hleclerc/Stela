#include "PointedData.h"
#include "PointerOn.h"
#include "Type.h"
#include "Room.h"
#include "Var.h"
#include "Cst.h"
#include "Ip.h"

Var::Var( Ref, const Var &var ) : inst( var.inst ), type( var.type ), flags( var.flags ) {
    inst->add_var_ptr( this );
}

Var::Var( Ref, Type *type, const Ptr<Inst> val ) : inst( val ), type( type ), flags( 0 ) {
}

Var::Var( Type *type, const Ptr<Inst> val ) : Var( type ) {
    inst->set( val );
}

Var::Var( Type *type ) : type( type ), flags( 0 ) {
    if ( type ) {
        inst = room( type );
        inst->add_var_ptr( this );
    }
}

Var::Var( SI32 val ) : Var( &ip->type_SI32, cst( val ) ) {
}

Var::Var( const Var &var ) {
    IP_ERROR( "Weird" );
}

Var &Var::operator=( const Var &var ) {
    // reassign
    inst->set( simplified( var.inst ) );
    return *this;
}

Var Var::ptr() {
    return Var( &ip->type_RawPtr, pointer_on( *this ) );
}

Var Var::ptd() {
    Type *type = &ip->type_SI32;
    return Var( Ref(), type, pointed_data( inst, type->size() ) );
}

void Var::write_to_stream( Stream &os ) const {
    os << inst;
}

int Var::size() const {
    return inst->size();
}

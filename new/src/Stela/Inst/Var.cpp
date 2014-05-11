#include "PointedData.h"
#include "PointerOn.h"
#include "Syscall.h"
#include "Type.h"
#include "Room.h"
#include "Var.h"
#include "Cst.h"
#include "Ip.h"

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

Var &Var::reassign( const Var &var ) {
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

Var syscall( Vec<Var> &inp ) {
    Vec<Ptr<Inst> > ch;
    for( int i = 0; i < inp.size(); ++i )
        ch << inp[ i ].inst->snapshot();
    Ptr<Inst> res = syscall( ch );
    res->add_dep( simplified( ip->sys_state.inst ) );
    ip->sys_state.reassign( Var( Ref(), &ip->type_Void, res ) );
    return { ip->type_ST, res };
}

#include "../System/SizeofIf.h"
#include "Uninitialized.h"
#include "Symbol.h"
#include "Type.h"
#include "Room.h"
#include "Cst.h"
#include "Ip.h"

Expr::Expr( const Expr &obj ) : inst( obj.inst ) {
    if ( inst ) ++inst->cpt_use;
}

Expr::Expr( Inst *inst ) : inst( inst ) {
    if ( inst ) ++inst->cpt_use;
}

#define DECL_BT( T ) Expr::Expr( T val ) : Expr( cst( ip->type_##T, SizeofIf<T,true>::val, &val ) ) {}
#include "DeclArytTypes.h"
#undef DECL_BT


Expr::Expr() : inst( uninitialized() ) {
    ++inst->cpt_use;
}

Expr::~Expr() {
    if ( inst and --inst->cpt_use <= 0 )
        delete inst;
}

Expr &Expr::operator=( const Expr &obj ) {
    if ( obj.inst )
        ++obj.inst->cpt_use;
    if ( inst and --inst->cpt_use <= 0 )
        delete inst;
    inst = obj.inst;
    return *this;
}

bool Expr::operator==( const Expr &expr ) const {
    if ( inst->same_cst( expr.inst ) )
        return true;
    return inst == expr.inst;
}

void Expr::write_to_stream( Stream &os ) const {
    if ( inst )
        os << *inst;
    else
        os << "NULL";
}

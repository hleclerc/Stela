#include "Uninitialized.h"
#include "Type.h"
#include "Room.h"
#include "Cst.h"
#include "Ip.h"

Expr::Expr( const Expr &obj ) {
    TODO;
}

Expr::Expr( Inst *inst ) : inst( inst ) {
    if ( inst ) ++inst->cpt_use;
}

Expr::Expr( SI32 val ) : Expr( room( cst( ip->type_SI32, 32, &val ) ) ) {
}

Expr::Expr() : inst( uninitialized() ) {
    ++inst->cpt_use;
}

Expr::~Expr() {
    if ( inst and --inst->cpt_use <= 0 )
        delete inst;
}

Expr &Expr::operator=( const Expr &obj ) {
    if ( uninitialized() ) {
        ++obj.inst->cpt_use;
        inst = obj.inst;
    } else
        TODO;
    return *this;
}

void Expr::write_to_stream( Stream &os ) const {
    if ( inst )
        os << *inst;
    else
        os << "NULL";
}

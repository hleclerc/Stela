#include "Inst.h"

inline Expr::Expr( const Expr &obj ) : inst( obj.inst ) {
    if ( inst ) ++inst->cpt_use;
}

inline Expr::Expr( Inst *obj ) : inst( obj ) {
    if ( inst ) ++inst->cpt_use;
}

inline Expr::~Expr() {
    if ( inst and --inst->cpt_use <= 0 )
        delete inst;
}

inline Expr &Expr::operator=( const Expr &obj ) {
    if ( obj.inst )
        ++obj.inst->cpt_use;
    if ( inst and --inst->cpt_use <= 0 )
        delete inst;
    inst = obj.inst;
    return *this;
}


#include "../Codegen/Codegen.h"
#include "../Codegen/OutReg.h"
#include "GlobalVariables.h"
#include "PointedValue.h"

/**
*/
struct PointedValue : Inst {
    virtual void write_dot( Stream &os ) const { os << "*"; }
    virtual int op_type() const { return ID_OP_PointedValue; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new PointedValue; }
    virtual Type *type() { return inp[ 0 ]->ptype(); }
    virtual void write( Codegen *c ) {
        if ( out_reg->type == ip->type_SI8  ) { c->on << *out_reg << " = v_SI8[ "  << Codegen::WriteOut{ c, inp[ 0 ] } << " >> 4 ];"; return; }
        if ( out_reg->type == ip->type_PI8  ) { c->on << *out_reg << " = v_PI8[ "  << Codegen::WriteOut{ c, inp[ 0 ] } << " >> 4 ];"; return; }
        if ( out_reg->type == ip->type_SI16 ) { c->on << *out_reg << " = v_SI16[ " << Codegen::WriteOut{ c, inp[ 0 ] } << " >> 5 ];"; return; }
        if ( out_reg->type == ip->type_PI16 ) { c->on << *out_reg << " = v_PI16[ " << Codegen::WriteOut{ c, inp[ 0 ] } << " >> 5 ];"; return; }
        if ( out_reg->type == ip->type_SI32 ) { c->on << *out_reg << " = v_SI32[ " << Codegen::WriteOut{ c, inp[ 0 ] } << " >> 6 ];"; return; }
        if ( out_reg->type == ip->type_PI32 ) { c->on << *out_reg << " = v_PI32[ " << Codegen::WriteOut{ c, inp[ 0 ] } << " >> 6 ];"; return; }
        if ( out_reg->type == ip->type_SI64 ) { c->on << *out_reg << " = v_SI64[ " << Codegen::WriteOut{ c, inp[ 0 ] } << " >> 7 ];"; return; }
        if ( out_reg->type == ip->type_PI64 ) { c->on << *out_reg << " = v_PI64[ " << Codegen::WriteOut{ c, inp[ 0 ] } << " >> 7 ];"; return; }

        if ( out_reg->type == ip->type_FP32 ) { c->on << *out_reg << " = v_FP32[ " << Codegen::WriteOut{ c, inp[ 0 ] } << " >> 6 ];"; return; }
        if ( out_reg->type == ip->type_FP64 ) { c->on << *out_reg << " = v_FP64[ " << Codegen::WriteOut{ c, inp[ 0 ] } << " >> 7 ];"; return; }

        TODO;
    }
};


Expr pointed_value( Expr ptr ) {
    PointedValue *res = new PointedValue;
    res->add_inp( ptr );
    return Inst::twin_or_val( res );
}

#include "../Codegen/Codegen_C.h"
#include "Store.h"

/**
*/
struct Store : Inst {
    virtual void write_dot( Stream &os ) const { os << "Store"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Store; }
    virtual Type *type() { return inp[ 0 ]->type(); }
    virtual void write( Codegen_C *cc ) {
        cc->on.write_beg();
        this->inp[ 0 ]->out_reg->write( cc, new_reg ) << " = (PI8 *)&";
        cc->write_out( this->inp[ 1 ] );
        cc->on.write_end( ";" );
    }
};

Expr store( Expr ptr, Expr val ) {
    Store *res = new Store();
    res->add_inp( ptr );
    res->add_inp( val );
    return res;
}


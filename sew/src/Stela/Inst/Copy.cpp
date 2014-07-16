#include "../Codegen/Codegen_C.h"
#include "Copy.h"

/**
*/
struct Copy : Inst {
    Copy() {} 
    virtual void write_dot( Stream &os ) const { os << "Copy"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Copy; }
    virtual Type *type() { return inp[ 0 ]->type(); }
    virtual void write( Codegen_C *cc, CC_SeqItemBlock **b ) {
        cc->on.write_beg();
        out_reg->write( cc, new_reg );
        *cc->os << " = ";
        inp[ 0 ]->out_reg->write( cc, false );
        cc->on.write_end( "; // cp" );
    }
};

Expr copy( Expr inp ) {
    Copy *res = new Copy;
    res->add_inp( inp );
    return res;
}


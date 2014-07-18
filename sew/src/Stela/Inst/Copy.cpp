#include "../Codegen/Codegen_C.h"
#include "Copy.h"

/**
*/
struct Copy : Inst {
    Copy() {} 
    virtual void write_dot( Stream &os ) const { os << "Copy"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Copy; }
    virtual Type *type() { return inp[ 0 ]->type(); }
    virtual bool will_write_code() const {
        return out_reg != inp[ 0 ]->out_reg;
    }

    virtual void get_constraints() {
        this->add_same_out( 0, this, -1, OPTIONNAL );
    }

    virtual void write( Codegen_C *cc, CC_SeqItemBlock **b ) {
        if ( out_reg == inp[ 0 ]->out_reg ) {
            // cc->on << "// cp " << ( out_reg ? out_reg->num : 666 );
            return;
        }
        cc->on.write_beg();
        if ( out_reg )
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


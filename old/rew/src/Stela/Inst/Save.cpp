#include "InstInfo_C.h"
#include "Save.h"

/**
*/
class Save : public Inst {
public:
    virtual void write_dot( Stream &os ) const { os << "save"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Save; }
    virtual int size() const { return inp[ 0 ]->size(); }
    virtual void write_to( Codegen_C *cc, int prec ) {
        cc->on << *IIC( this )->out_reg << " = " << cc->code( inp[ 0 ] ) << ";";
    }
    virtual void inp_type_proposition( Type *type, int ninp ) {
        out_type_proposition( type );
    }
    virtual void out_type_proposition( Type *type ) {
        Inst::out_type_proposition( type );
        inp[ 0 ]->out_type_proposition( type );
    }
};

Expr save( Expr inp ) {
    Save *res = new Save;
    res->add_inp( inp );
    return res;
}

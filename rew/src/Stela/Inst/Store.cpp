#include "InstInfo_C.h"
#include "Store.h"
#include "Ip.h"

/**
  *inp[ 0 ] = inp[ 1 ]
*/
class Store : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "store";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Store;
    }
    virtual int size() const {
        return 0;
    }
    virtual void write_to( Codegen_C *cc, int prec ) {
        cc->on << "R" << IIC( inp[ 0 ] )->num_reg << " = " << cc->code( inp[ 1 ] ) << ";";
    }
    virtual void inp_type_proposition( Type *type, int ninp ) {
        if ( ninp == 1 )
            inp[ 0 ]->val_type_proposition( type );
    }
    virtual void update_out_type() {
        inp[ 0 ]->out_type_proposition( ip->type_ST );
    }

};

Expr store( Expr ptr, Expr val ) {
    Store *res = new Store;
    res->add_inp( ptr );
    res->add_inp( val );
    return res;
}

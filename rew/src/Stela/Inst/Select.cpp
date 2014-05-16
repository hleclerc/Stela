#include "Select.h"
#include "Ip.h"

/**
  inp[ 0 ] ? inp[ 1 ] : inp[ 2 ]
*/
class Select : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "select";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Select;
    }
    virtual int size() const {
        return inp[ 1 ]->size();
    }
    virtual Expr _simplified() {
        Bool res;
        if ( inp[ 0 ]->val_if( ip->cur_cond() )->get_val( res ) )
            return res ? inp[ 1 ] : inp[ 0 ];
        return 0;
    }
};

Expr select( Expr cond, Expr ok, Expr ko ) {
    Select *res = new Select;
    res->add_inp( cond );
    res->add_inp( ok );
    res->add_inp( ko );
    return res;
}

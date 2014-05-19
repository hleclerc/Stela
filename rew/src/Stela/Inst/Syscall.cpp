#include "Syscall.h"
#include "Ip.h"

/**
*/
class Syscall : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "sys_state";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Syscall;
    }
    virtual int size() const {
        return ip->type_ST->size();
    }
    virtual Expr _get_val() {
        return this;
    }
};

Expr syscall( const Vec<Expr> &inp, Expr state ) {
    Syscall *res = new Syscall;
    for( Expr i : inp )
        res->add_inp( i );
    res->add_dep( state );
    return res;
}

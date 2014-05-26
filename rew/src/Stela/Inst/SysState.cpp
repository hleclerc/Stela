#include "SysState.h"

/**
*/
class SysState : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "sys_state";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new SysState;
    }
    virtual int size() const {
        return 0;
    }
    virtual Expr _get_val( int len ) {
        return this;
    }
};

Expr sys_state() {
    return new SysState;
}

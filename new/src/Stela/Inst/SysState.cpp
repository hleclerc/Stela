#include "SysState.h"
#include "Ip.h"

/**
*/
class SysState : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "sys_state";
    }
    virtual int size() const {
        return 0;
    }
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const {
        return new SysState;
    }
    virtual void set( Ptr<Inst> val ) {
        val->add_dep( simplified( ip->sys_state.inst ) );
        ip->sys_state.inst = val;
    }
    virtual void write_to( CodeGen_C *cc ) const {
    }
    virtual void write_1l_to( CodeGen_C *cc ) const {
    }
};


Ptr<Inst> sys_state() {
    return new SysState;
}

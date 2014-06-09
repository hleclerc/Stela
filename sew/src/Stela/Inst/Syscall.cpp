#include "Syscall.h"
#include "Ip.h"

/**
*/
struct Syscall : Inst {
    Syscall() {} 
    virtual void write_dot( Stream &os ) { os << "Syscall"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Syscall(); }
    virtual Type *type() { return ip->type_ST; }

};

Expr syscall( Vec<Expr> inp ) {
    Syscall *res = new Syscall();
    ++Inst::cur_op_id;
    for( Expr i : inp ) {
        i->add_store_dep( res );
        res->add_inp( i );
    }
    res->add_dep( ip->sys_state );
    ip->sys_state = res;
    return res;
}


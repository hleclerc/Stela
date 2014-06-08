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

Expr syscall() {
    Syscall *res = new Syscall();
    return res;
}


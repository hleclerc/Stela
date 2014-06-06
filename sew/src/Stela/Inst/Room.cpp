#include "Room.h"
#include "Ip.h"

/**
*/
struct Room : Inst {
    virtual void write_dot( Stream &os ) { os << "&"; }
    virtual void write_to_stream( Stream &os, int prec = -1 ) {
        os << "&(" << val << ")";
    }

    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Room; }
    virtual Type *type() { return ip->type_RawPtr; }

    Expr val;
};

Inst *room( Expr expr ) {
    Room *res = new Room;
    res->val = expr.inst;
    return res;
}


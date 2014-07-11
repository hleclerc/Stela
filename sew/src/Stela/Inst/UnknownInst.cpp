#include "UnknownInst.h"

/**
*/
struct UnknownInst : Inst {
    UnknownInst( Type *type, int num ) : out_type( type ), num( num ) {}
    virtual void write_dot( Stream &os ) const { os << "unk_" << num; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new UnknownInst( out_type, num ); }
    virtual Type *type() { return out_type; }
    Type *out_type;
    int num;
};

Expr unknown_inst( Type *type, int num ) {
    UnknownInst *res = new UnknownInst( type, num );
    return res;
}


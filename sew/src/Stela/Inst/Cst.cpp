#include "Type.h"
#include "Cst.h"

/**
*/
struct Cst : Inst {
    virtual void write_dot( Stream &os ) {
        out_type->write_to_stream( os, data.ptr(), len );
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        Cst *res = new Cst;
        res->out_type = out_type;
        res->data = data;
        res->knwn = knwn;
        res->len = len;
        return res;
    }
    virtual Type *type() { return out_type; }

    Type *out_type;
    Vec<PI8> data;
    Vec<PI8> knwn;
    int len;
};

Inst *cst( Type *type, int len, void *data, void *knwn ) {
    Cst *res = new Cst;
    res->out_type = type;
    res->len = len;

    int sb = ( len + 7 ) / 8;
    if ( data )
        res->data = Vec<PI8>( (PI8 *)data, (PI8 *)data + sb );
    else
        res->data.resize( sb, 0 );

    if ( knwn )
        res->knwn = Vec<PI8>( (PI8 *)knwn, (PI8 *)knwn + sb );
    else
        res->knwn.resize( sb, 0 );;

    return res;
}


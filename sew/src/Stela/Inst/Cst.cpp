#include <string.h>
#include "Type.h"
#include "Cst.h"
#include "Ip.h"

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
    virtual Type *type() {
        return out_type;
    }
    virtual bool get_val( Type *type, void *dst ) const {
        if ( type->size() < 0 )
            return false;
        if ( type == out_type ) {
            memcpy( dst, data.ptr(), data.size() );
            return true;
        }
        //if ( type == ip->type_Bool ) {
        //    return out_type->conv( dst, type, data.ptr() );
        //}
        TODO;
        return false;
    }
    virtual bool same_cst( const Inst *inst ) const {
        return inst->emas_cst( this );
    }
    virtual bool emas_cst( const Inst *inst ) const {
        const Cst *c = static_cast<const Cst *>( inst );
        return c->out_type == out_type and c->data == data and c->knwn == knwn and c->len == len;
    }

    Type *out_type;
    Vec<PI8> data;
    Vec<PI8> knwn;
    int len;
};

Expr cst( Type *type, int len, void *data, void *knwn ) {
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


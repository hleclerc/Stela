#include "../System/Memcpy.h"
#include "../System/dcast.h"
#include <string.h>
#include "Type.h"
#include "Cst.h"
#include "Ip.h"

/**
*/
struct Cst : Inst {
    virtual void write_dot( Stream &os ) {
        #define DECL_BT( T ) \
            if ( out_type == ip->type_##T ) { os << *reinterpret_cast<T *>( data.ptr() ); return; }
        #include "DeclArytTypes.h"
        #undef DECL_BT
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
    virtual Expr size() {
        return len;
    }
    virtual Expr _simp_repl_bits( Expr off, Expr val ) {
        if ( Cst *c = dcast( val.inst ) ) {
            SI32 voff;
            if ( off->get_val( ip->type_SI32, &voff ) ) {
                SI32 vlen;
                if ( val->size()->get_val( ip->type_SI32, &vlen ) ) {
                    Cst *res = new Cst;
                    res->out_type = out_type;
                    res->len = len;
                    res->knwn = knwn;
                    res->data = data;
                    vlen = std::min( vlen, SI32( len - voff ) );
                    memcpy_bit( res->data.ptr(), voff, c->data.ptr(), 0, vlen );
                    memset_bit( res->knwn.ptr(), voff, true, vlen );
                    return res;
                }
            }
        }
        return (Inst *)0;
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


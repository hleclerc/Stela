#include "../Codegen/Codegen_C.h"
#include "../System/Memcpy.h"
#include "../System/dcast.h"
#include <string.h>
#include <typeinfo>
#include "Type.h"
#include "Cst.h"
#include "Ip.h"

/**
*/
struct Cst : Inst {
    virtual void write_dot( Stream &os ) const {
        //
        if ( out_type == ip->type_SI8 ) {
            os << (int)*reinterpret_cast<const SI8 *>( data.ptr() );
            return;
        }
        if ( out_type == ip->type_PI8 ) {
            os << (int)*reinterpret_cast<const PI8 *>( data.ptr() );
            return;
        }
        //
        #define DECL_BT( T ) \
            if ( out_type == ip->type_##T ) { os << *reinterpret_cast<const T *>( data.ptr() ); return; }
        #include "DeclArytTypes.h"
        #undef DECL_BT
        //
        if ( out_type == ip->type_Type ) {
            ST d = *reinterpret_cast<const SI64 *>( data.ptr() );
            os << *reinterpret_cast<Type *>( d );
            return;
        }
        if ( out_type == ip->type_Void ) {
            os << "void";
            return;
        }
        if ( out_type == ip->type_Error ) {
            os << "error";
            return;
        }
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

    template<class T>
    bool _get_val( T *dst ) const {
        #define DECL_BT( OT ) if ( out_type == ip->type_##OT ) { *dst = *reinterpret_cast<const OT *>( data.ptr() ); return true; }
        #include "DeclArytTypes.h"
        #undef DECL_BT
        return false;
    }

    virtual bool get_val( Type *type, void *dst ) const {
        if ( type == out_type ) {
            memcpy( dst, data.ptr(), data.size() );
            return true;
        }
        if ( type->aryth and out_type->aryth ) {
            #define DECL_BT( RT ) if ( type == ip->type_##RT ) return _get_val( reinterpret_cast<RT *>( dst ) );
            #include "DeclArytTypes.h"
            #undef DECL_BT
        }

        PRINT( *type );
        PRINT( *out_type );
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
    virtual Expr _simp_repl_bits( Expr off, Expr val ) {
        if ( Cst *c = dcast( val.inst ) ) {
            SI32 voff;
            if ( off->get_val( ip->type_SI32, &voff ) ) {
                SI32 vlen = val->size();
                Cst *res = new Cst;
                res->out_type = out_type;
                res->len = std::max( len, voff + vlen );
                res->data.resize( res->sb() );
                res->knwn.resize( res->sb() );
                // old values
                memcpy( res->data.ptr(), data.ptr(), res->sb() );
                memcpy( res->knwn.ptr(), knwn.ptr(), res->sb() );
                // new (replaced) ones
                memcpy_bit( res->data.ptr(), voff, c->data.ptr(), 0, vlen );
                memset_bit( res->knwn.ptr(), voff, true, vlen );
                return res;
            }
        }
        return (Inst *)0;
    }
    virtual Expr _simp_slice( Type *dst, Expr off ) {
        SI32 voff;
        if ( off->get_val( ip->type_SI32, &voff ) ) {
            Cst *res = new Cst;
            res->out_type = dst;
            res->len = len - voff;
            res->knwn.resize( ( res->len + 7 ) / 8 );
            res->data.resize( ( res->len + 7 ) / 8 );
            // vlen = std::min( vlen, SI32( len - voff ) );
            memcpy_bit( res->data.ptr(), 0, data.ptr(), voff, res->len );
            memcpy_bit( res->knwn.ptr(), 0, knwn.ptr(), voff, res->len );
            return res;
        }
        return (Inst *)0;
    }
    int sb() const {
        return ( len + 7 ) / 8;
    }
    virtual bool need_a_register() {
        return len;
    }
    virtual void write( Codegen_C *cc ) {
        if ( not out_reg )
            return;
        if ( out_type->aryth ) {
            cc->on.write_beg();
            out_reg->write( cc, new_reg ) << " = ";
            write_dot( *cc->os );
            cc->on.write_end( ";" );
        } else {
            // decl
            if ( new_reg ) {
                cc->on.write_beg();
                out_reg->write( cc, new_reg );
                cc->on.write_end( ";" );
            }

            // fill in
            for( int i = 0; i < sb(); ++i ) {
                if ( knwn[ i ] ) {
                    cc->on.write_beg();
                    out_reg->write( cc, false ) << ".data[ " << i << " ] = " << (int)data[ i ];
                    cc->on.write_end( ";" );
                }
            }
        }
    }

    Type *out_type;
    Vec<PI8> data;
    Vec<PI8> knwn;
    int len;
};

Expr cst( Type *type, int len, const void *data, const void *knwn ) {
    Cst *res = new Cst;
    res->out_type = type;
    res->len = len;

    if ( len >= 0 ) {
        int sb = ( len + 7 ) / 8;
        if ( data )
            res->data = Vec<PI8>( (PI8 *)data, (PI8 *)data + sb );
        else
            res->data.resize( sb, 0 );

        if ( knwn )
            res->knwn = Vec<PI8>( (PI8 *)knwn, (PI8 *)knwn + sb );
        else
            res->knwn.resize( sb, 0 );
    }

    return res;
}


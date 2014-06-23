#include "Uninitialized.h"
#include "IpSnapshot.h"
#include "BoolOpSeq.h"
#include "ReplBits.h"
#include "Select.h"
#include "Store.h"
#include "Slice.h"
#include "Room.h"
#include "Type.h"
#include "Ip.h"

/**
*/
struct Room : Inst {
    Room() : creation_date( IpSnapshot::cur_date ) {}
    virtual void write_dot( Stream &os ) { os << "&"; }
    virtual void write_to_stream( Stream &os, int prec = -1 ) {
        //Type *t = type();
        //if ( t and t != ip->type_Type ) { os << *t; os << "{"; }
        os << "&(" << val << ")";
        //if ( t and t != ip->type_Type ) os << "}";
    }

    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Room; }
    virtual Type *type() { return ip->ptr_for( val->type() ); }
    virtual void set( Expr obj, const BoolOpSeq &cond ) {
        if ( flags & CONST )
            return ip->disp_error( "attempting to modify a const value" );
        if ( IpSnapshot *is = ip->cur_ip_snapshot ) {
            if ( creation_date < is->date and not is->rooms.count( this ) )
                is->rooms[ this ] = val;
        }
        val = select( cond, repl_bits( val->simplified( cond ), 0, obj->simplified( cond ) ), val->simplified( not cond ) );
    }
    virtual Expr get( const BoolOpSeq &cond ) {
        return val->simplified( cond );
    }

    virtual void _mk_store_dep( Inst *dst ) {
        Expr st = store( this, val );
        for( Expr &d : future_dep )
            st->add_dep( d ); // inst that use stored value must be done before thet change
        dst->add_dep( st ); // dst depends on store

        future_dep.resize( 0 );
        future_dep << dst;

        val->add_store_dep( dst );
    }

    Vec<Expr> future_dep;
    int creation_date;
    Expr val;
};

Expr room( Expr expr, int flags ) {
    Room *res = new Room;
    res->flags = flags;
    res->val = expr.inst;
    return res;
}

Expr room() {
    return room( uninitialized() );
}

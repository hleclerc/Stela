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
    virtual void write_dot( Stream &os ) const { os << "&"; }
    virtual void write_to_stream( Stream &os, int prec = -1 ) {
        //Type *t = type();
        //if ( t and t != ip->type_Type ) { os << *t; os << "{"; }
        os << "&(" << val << ")";
        //if ( t and t != ip->type_Type ) os << "}";
    }

    virtual Expr forced_clone( Vec<Expr> &created ) const { Room *res = new Room; res->val = val; return res; }
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
    virtual bool referenced_more_than_one_time() const {
        return cpt_use != 1 or val->cpt_use != 1;
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

    virtual bool will_write_code() const {
        return false;
    }

    virtual void write( Codegen_C *cc, CC_SeqItemBlock **b ) {
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

#include "Uninitialized.h"
#include "BoolOpSeq.h"
#include "Select.h"
#include "Room.h"
#include "Ip.h"

/**
*/
struct Room : Inst {
    enum {
        CONST = 1
    };

    Room() : flags( 0 ) {}
    virtual void write_dot( Stream &os ) { os << "&"; }
    virtual void write_to_stream( Stream &os, int prec = -1 ) {
        os << "&(" << val << ")";
    }

    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Room; }
    virtual Type *type() { return ip->type_RawPtr; }
    virtual void set( Expr obj, const BoolOpSeq &cond ) {
        if ( flags & CONST )
            return ip->disp_error( "attempting to modify a const value" );
        //for( IpSnapshot *is : ip->snapshots ) {
        //    if ( date < is->date ) {
        //        if ( not is->changed.count( this ) )
        //            is->changed[ this ] = this->val;
        //    }
        //}
        val = select( cond, obj->simplified( cond ), val->simplified( not cond ) );
    }
    virtual Expr get( const BoolOpSeq &cond ) {
        return val->simplified( cond );
    }

    Expr val;
    int  flags;
};

Expr room( Expr expr ) {
    Room *res = new Room;
    res->val = expr.inst;
    return res;
}

Expr room() {
    return room( uninitialized() );
}

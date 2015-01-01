#include "ParsingContext.h"
#include "IpSnapshot.h"
#include "ReplBits.h"
#include "Select.h"
#include "Room.h"

struct Room : Inst {
    Room( Expr val, bool cons ) : val( val ), cons( cons ), creation_date( IpSnapshot::cur_date ) {
        in_an_ip_snapshot = false;
    }
    virtual ~Room() {
        if ( in_an_ip_snapshot )
            for( IpSnapshot *is = ip->ip_snapshot; is; is = is->prev )
                is->rooms.erase( this );
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
         return new Room( val, cons );
    }
    virtual void write_to_stream( Stream &os, int prec = -1 ) {
        os << "&(" << val << ")";
    }
    virtual void write_dot( Stream &os ) const {
        os << "&";
    }
    virtual void _mk_store_dep( Inst *dst ) {
        TODO;
    }
    virtual void set( Expr obj, Expr cond ) {
        // the same value ?
        if ( obj == val )
            return;
        // forbidden ?
        if ( cons )
            return ip->pc->disp_error( "attempting to modify a const value" );
        //
        if ( IpSnapshot *is = ip->ip_snapshot ) {
            if ( creation_date < is->date ) {
                is->rooms.insert( std::make_pair( this, val ) );
                in_an_ip_snapshot = true;
            }
        }
        val = select( cond, repl_bits( val->simplified( cond ), SI64( 0 ), obj->simplified( cond ) ), val->simplified( not cond ) );
    }
    virtual Expr get( Expr cond ) {
        return val->simplified( cond );
    }
    virtual Type *ptype() {
        return val ? val->type() : 0;
    }
    virtual Type *type() {
        TODO;
        return 0;
    }

    Expr val;
    bool cons;
    int  creation_date;
    bool in_an_ip_snapshot;
};

Expr room( Expr val, bool cons ) {
    return new Room( val, cons );
}

#include "ParsingContext.h"
#include "ReplBits.h"
#include "Select.h"
#include "Room.h"

struct Room : Inst {
    Room( Expr val, bool cons ) : val( val ), cons( cons ) {
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
         return new Room( val, cons );
    }
    virtual void write_dot( Stream &os ) const {
        os << "&";
    }
    virtual void _mk_store_dep( Inst *dst ) {
        TODO;
    }
    virtual void set( Expr obj, Expr cond ) {
        if ( cons )
            return ip->disp_error( "attempting to modify a const value" );
        if ( IpSnapshot *is = ip->ip_snapshot ) {
            TODO;
            PRINT( is );
            //if ( creation_date < is->date and not is->rooms.count( this ) )
            //    is->rooms[ this ] = val;
        }
        val = select( cond, repl_bits( val->simplified( cond ), SI64( 0 ), obj->simplified( cond ) ), val->simplified( not cond ) );
    }
    virtual Expr get( Expr cond ) {
        return val->simplified( cond );
    }
    Expr val;
    bool cons;
};

Expr room( Expr val, bool cons ) {
    return new Room( val, cons );
}

#include "../Codegen/Codegen_C.h"
#include "IpSnapshot.h"
#include "Syscall.h"
#include "Ip.h"

/**
*/
struct Syscall : Inst {
    Syscall() {} 
    virtual void write_dot( Stream &os ) { os << "Syscall"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Syscall(); }
    virtual Type *type() { return ip->type_ST; }
    virtual void write( Codegen_C *cc, CC_SeqItemBlock **b ) {
        cc->on.write_beg();
        if ( out_reg )
            out_reg->write( cc, new_reg ) << " = ";
        *cc->os << "syscall( ";
        for( int i = 0; i < inp.size(); ++i ) {
            if ( i )
                *cc->os << ", ";
            cc->write_out( inp[ i ] );
        }
        cc->on.write_end( " );" );
    }


};

Expr syscall( Vec<Expr> inp, const BoolOpSeq &cond ) {
    Syscall *res = new Syscall();
    ++Inst::cur_op_id;
    for( Expr i : inp ) {
        if ( i.error() )
            return ip->error_var();
        i->add_store_dep( res );
        res->add_inp( i );
    }
    res->add_dep( ip->sys_state->get( cond ) );

    ip->sys_state->set( res, cond );
    return res;
}


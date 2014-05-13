#include "../Inst/Op.h"
#include "../Inst/Ip.h"
#include "InstInfo_C.h"

InstInfo_C::InstInfo_C( const Ptr<Inst> &inst_false ) : when( inst_false ) {
    num_reg  = -1;
    sched_in = 0;
}

bool InstInfo_C::add_when_possibility( const Ptr<Inst> &cond ) {
    if ( when->true_if( cond ) )
        return true;
    when = op( &ip->type_Bool, &ip->type_Bool, when, &ip->type_Bool, cond, Op_or() );
    return false;
}

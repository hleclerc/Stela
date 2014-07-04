#include "../Inst/CppRegConstraint.h"
#include "CC_SeqItemExpr.h"
#include "Codegen_C.h"

CC_SeqItemExpr::CC_SeqItemExpr( Expr expr, CC_SeqItemBlock *parent_block ) : CC_SeqItem( parent_block, parent_block ), expr( expr ) {
}

void CC_SeqItemExpr::write( Codegen_C *cc ) {
    CC_SeqItemBlock *b[ ext.size() ];
    for( int i = 0; i < ext.size(); ++i )
        b[ i ] = ext[ i ].ptr();
    expr->write( cc, b );
}

void CC_SeqItemExpr::get_constraints( CppRegConstraint &reg_constraints ) {
    expr->get_constraints( reg_constraints );
    for( AutoPtr<CC_SeqItemBlock> &b : ext )
        b->get_constraints( reg_constraints );
}

void CC_SeqItemExpr::assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints ) {
    if ( expr->need_a_register() ) {
        // constrained ?
        ++Inst::cur_op_id;
        expr->out_reg = reg_constraints.compulsory_reg( expr );
        // else,
        if ( not expr->out_reg )
            expr->out_reg = cc->new_out_reg( expr->type() );
        //
        expr->out_reg->provenance << parent_block;
    }
    for( AutoPtr<CC_SeqItemBlock> &b : ext )
        b->assign_reg( cc, reg_constraints );
}

bool CC_SeqItemExpr::ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond ) {
    return parent->ch_followed_by_something_to_execute( nb_evicted_blocks, this, cond );
}

bool CC_SeqItemExpr::non_void() {
    return expr->will_write_code();
}

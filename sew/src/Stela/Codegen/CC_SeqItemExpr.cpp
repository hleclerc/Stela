#include "CppGetConstraint.h"
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

void CC_SeqItemExpr::get_constraints( CppGetConstraint &context ) {
    context.seq << expr.inst;

    expr->get_constraints();
    for( AutoPtr<CC_SeqItemBlock> &b : ext )
        b->get_constraints( context );

    // update cur_live_outputs
    for( Expr i : expr->inp )
        if( --context.cur_live_outputs[ i.inst ] == 0 )
            context.cur_live_outputs.erase( i.inst );

    for( auto cli : context.cur_live_outputs )
        expr->add_diff_out( -1, cli.first, -1, Inst::COMPULSORY );

    if ( int n = expr->nb_inp_parents() )
        context.cur_live_outputs[ expr.inst ] = n;
}

bool CC_SeqItemExpr::ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond ) {
    return parent->ch_followed_by_something_to_execute( nb_evicted_blocks, this, cond );
}

bool CC_SeqItemExpr::non_void() {
    return expr->will_write_code();
}

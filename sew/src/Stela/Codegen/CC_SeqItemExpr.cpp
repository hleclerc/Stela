#include "CC_SeqItemExpr.h"
#include "Codegen_C.h"

CC_SeqItemExpr::CC_SeqItemExpr( Expr expr, CC_SeqItemBlock *parent_block ) : CC_SeqItem( parent_block, parent_block ), expr( expr ) {
    expr->cc_item_expr = this;
}

void CC_SeqItemExpr::write( Codegen_C *cc ) {
    CC_SeqItemBlock *b[ ext.size() ];
    for( int i = 0; i < ext.size(); ++i )
        b[ i ] = ext[ i ].ptr();
    expr->write( cc, b );
}

bool CC_SeqItemExpr::ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond ) {
    return parent->ch_followed_by_something_to_execute( nb_evicted_blocks, this, cond );
}

bool CC_SeqItemExpr::visit( Visitor &v ) {
    return v( *this );
}


bool CC_SeqItemExpr::non_void() {
    return expr->will_write_code();
}

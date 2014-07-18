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

struct VisitorTo_Stage1 : CC_SeqItem::Visitor {
    virtual bool operator()( CC_SeqItemExpr &ce ) {
        // std::cout << "   1 " << ce.expr << "\n";
        ce.op_id = CC_SeqItem::cur_op_id;
        return &ce != goal;
    }
    CC_SeqItemExpr *goal;
};

struct VisitorTo_Stage2 : CC_SeqItem::Visitor {
    virtual bool operator()( CC_SeqItemExpr &ce ) {
        // std::cout << "   2 " << ce.expr << "\n";
        if ( &ce == goal ) {
            reached = true;
            return false;
        }
        return ce.op_id == CC_SeqItem::cur_op_id ? (*v)( ce ) : true;
    }
    CC_SeqItemExpr *goal;
    Visitor        *v;
    bool            reached;
};

bool CC_SeqItemExpr::following_visit_to( Visitor &v, CC_SeqItemExpr *goal ) {
    ++cur_op_id;

    // stage 1: mark CC_SeqItemExpr backward
    VisitorTo_Stage1 vtm_1;
    vtm_1.goal = this;
    goal->preceding_visit( vtm_1 );

    // stage 2: visit marked CC_SeqItemExpr forward
    VisitorTo_Stage2 vtm_2;
    vtm_2.goal    = goal;
    vtm_2.v       = &v;
    vtm_2.reached = false;
    following_visit( vtm_2 );
    return vtm_2.reached;
}

bool CC_SeqItemExpr::preceding_visit_to( Visitor &v, CC_SeqItemExpr *goal ) {
    ++cur_op_id;

    // stage 1: mark CC_SeqItemExpr backward
    VisitorTo_Stage1 vtm_1;
    vtm_1.goal = this;
    goal->following_visit( vtm_1 );

    // stage 2: visit marked CC_SeqItemExpr forward
    VisitorTo_Stage2 vtm_2;
    vtm_2.goal    = goal;
    vtm_2.v       = &v;
    vtm_2.reached = false;
    preceding_visit( vtm_2 );
    return vtm_2.reached;
}

bool CC_SeqItemExpr::visit( Visitor &v, bool forward ) {
    return v( *this );
}


bool CC_SeqItemExpr::non_void() {
    return expr->will_write_code();
}

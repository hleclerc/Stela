#include "CC_SeqItem.h"

CC_SeqItem::CC_SeqItem( CC_SeqItem *parent, CC_SeqItemBlock *parent_block ) : parent_block( parent_block ), parent( parent ) {
}

CC_SeqItem::~CC_SeqItem() {
}

void CC_SeqItem::get_glo_cond_and_seq_of_sub_blocks( Vec<CC_SeqItemBlock *> &res, const BoolOpSeq &cond ) {
}

bool CC_SeqItem::following_visit( Visitor &v, CC_SeqItem *avoid ) {
    return parent ? parent->following_visit( v, this ) : true;
}

bool CC_SeqItem::contains( CC_SeqItemExpr *expr ) {
    struct Contains : Visitor {
        virtual bool operator()( CC_SeqItemExpr &ce ) {
            if ( &ce != expr )
                return true;
            found = true;
            return false;
        }
        CC_SeqItemExpr *expr;
        bool found;
    };
    Contains c;
    c.expr = expr;
    c.found = false;
    visit( c );
    return c.found;
}

bool CC_SeqItem::preceding_visit( Visitor &v, CC_SeqItem *avoid ) {
    return parent ? parent->preceding_visit( v, this ) : true;
}

bool CC_SeqItem::visit( Visitor &v ) {
    return true;
}

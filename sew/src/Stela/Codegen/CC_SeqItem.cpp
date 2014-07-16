#include "CC_SeqItem.h"

CC_SeqItem::CC_SeqItem( CC_SeqItem *parent, CC_SeqItemBlock *parent_block ) : parent_block( parent_block ), parent( parent ) {
}

CC_SeqItem::~CC_SeqItem() {
}

void CC_SeqItem::get_glo_cond_and_seq_of_sub_blocks( Vec<CC_SeqItemBlock *> &res, const BoolOpSeq &cond ) {
}

bool CC_SeqItem::visit( Visitor &v ) {
    return true;
}

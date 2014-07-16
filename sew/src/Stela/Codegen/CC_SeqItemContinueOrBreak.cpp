#include "CC_SeqItemContinueOrBreak.h"
#include "CC_SeqItemBlock.h"
#include "Codegen_C.h"

CC_SeqItemContinueOrBreak::CC_SeqItemContinueOrBreak( bool cont, CC_SeqItemBlock *parent_block ) : CC_SeqItem( parent_block, parent_block ), cont( cont ) {
}

void CC_SeqItemContinueOrBreak::CC_SeqItemContinueOrBreak::write( Codegen_C *cc ) {
    cc->on << ( cont ? "continue;" : "break;" );
}

bool CC_SeqItemContinueOrBreak::ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond ) {
    ERROR( "weird -> ContinueOrBreak do not have children" );
    return false;
}

bool CC_SeqItemContinueOrBreak::non_void() {
    return true;
}

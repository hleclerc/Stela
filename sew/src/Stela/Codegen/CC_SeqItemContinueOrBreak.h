#ifndef CC_SEQITEMCONTINUEORBREAK_H
#define CC_SEQITEMCONTINUEORBREAK_H

#include "CC_SeqItem.h"

/**
*/
struct CC_SeqItemContinueOrBreak : CC_SeqItem {
    CC_SeqItemContinueOrBreak( bool cont, CC_SeqItemBlock *parent_block );
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppGetConstraint &context );
    virtual bool ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond );
    virtual void write_to_stream( Stream &os ) { os << "cont/break"; }
    virtual bool non_void();
    bool cont;
};


#endif // CC_SEQITEMCONTINUEORBREAK_H

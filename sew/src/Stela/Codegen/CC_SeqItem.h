#ifndef CC_SEQITEM_H
#define CC_SEQITEM_H

#include "../System/SplittedVec.h"
#include "../System/AutoPtr.h"
#include "../Inst/BoolOpSeq.h"
class CppGetConstraint;
class CC_SeqItemBlock;
class Codegen_C;

struct CC_SeqItem {
    CC_SeqItem( CC_SeqItem *parent, CC_SeqItemBlock *parent_block );
    virtual ~CC_SeqItem();
    virtual void write( Codegen_C *cc ) = 0;
    virtual void get_constraints( CppGetConstraint &context ) = 0;
    virtual void get_glo_cond_and_seq_of_sub_blocks( Vec<CC_SeqItemBlock *> &seq, const BoolOpSeq &cond );
    virtual bool ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond ) = 0;
    virtual void write_to_stream( Stream &os ) = 0;
    virtual bool non_void() = 0;
    CC_SeqItemBlock *parent_block;
    CC_SeqItem *parent;

    BoolOpSeq glo_cond; ///< filled by get_glo_cond_and_seq_of_sub_blocks
};



#endif // CC_SEQITEM_H

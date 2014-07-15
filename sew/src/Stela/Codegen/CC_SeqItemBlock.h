#ifndef CC_SEQITEMBLOCK_H
#define CC_SEQITEMBLOCK_H

#include "CC_SeqItem.h"

/**
*/
struct CC_SeqItemBlock : CC_SeqItem {
    CC_SeqItemBlock();
    virtual ~CC_SeqItemBlock();
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppGetConstraint &context );
    virtual void get_glo_cond_and_seq_of_sub_blocks( Vec<CC_SeqItemBlock *> &seq, const BoolOpSeq &cond );
    virtual bool ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond );
    virtual void write_to_stream( Stream &os ) { os << "block"; }
    virtual bool non_void();

    bool contains_a_cont_or_break();
    void insert_before( CC_SeqItem *iter, CC_SeqItem *item );

    SplittedVec<AutoPtr<CC_SeqItem>,8> seq;
    Vec<CppOutReg *> reg_to_decl;
    CC_SeqItemBlock *sibling;

    mutable int n;
};


#endif // CC_SEQITEMBLOCK_H

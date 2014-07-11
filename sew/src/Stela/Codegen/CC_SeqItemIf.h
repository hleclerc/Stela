#ifndef CC_SEQITEMIF_H
#define CC_SEQITEMIF_H

#include "CC_SeqItemBlock.h"

/**
*/
struct CC_SeqItemIf : CC_SeqItem {
    CC_SeqItemIf( CC_SeqItemBlock *parent );
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppGetConstraint &context );
    virtual void get_glo_cond_and_seq_of_sub_blocks( Vec<CC_SeqItemBlock *> &seq, const BoolOpSeq &cond );
    virtual bool ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond );
    virtual void write_to_stream( Stream &os ) { os << "if"; }
    virtual bool non_void();

    CC_SeqItemBlock seq[ 2 ];
    BoolOpSeq cond;
    int cur_seq;
};

#endif // CC_SEQITEMIF_H

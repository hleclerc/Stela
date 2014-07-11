#ifndef CC_SEQITEMEXPR_H
#define CC_SEQITEMEXPR_H

#include "CC_SeqItemBlock.h"

struct CC_SeqItemExpr : CC_SeqItem {
    CC_SeqItemExpr( Expr expr, CC_SeqItemBlock *parent_block );
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppGetConstraint &context );
    virtual bool ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond );
    virtual void write_to_stream( Stream &os ) { os << expr; }
    virtual bool non_void();
    Vec<AutoPtr<CC_SeqItemBlock> > ext;
    Expr expr;
};

#endif // CC_SEQITEMEXPR_H

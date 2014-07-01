#ifndef CC_SEQITEM_H
#define CC_SEQITEM_H

#include "../System/SplittedVec.h"
#include "../System/AutoPtr.h"
#include "../Inst/BoolOpSeq.h"
class CppRegConstraint;
class CC_SeqItemBlock;
class Codegen_C;

struct CC_SeqItem {
    CC_SeqItem( CC_SeqItem *parent, CC_SeqItemBlock *parent_block );
    virtual ~CC_SeqItem();
    virtual void write( Codegen_C *cc ) = 0;
    virtual void get_constraints( CppRegConstraint &reg_constraints ) = 0;
    virtual void assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints ) = 0;
    virtual void get_seq_of_sub_blocks( Vec<std::pair<BoolOpSeq,CC_SeqItemBlock *> > &seq, const BoolOpSeq &cond );
    CC_SeqItemBlock *parent_block;
    CC_SeqItem *parent;
};

struct CC_SeqItemBlock : CC_SeqItem {
    CC_SeqItemBlock();
    virtual ~CC_SeqItemBlock();
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppRegConstraint &reg_constraints );
    virtual void assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints );
    virtual void get_seq_of_sub_blocks( Vec<std::pair<BoolOpSeq,CC_SeqItemBlock *> > &seq, const BoolOpSeq &cond );
    SplittedVec<AutoPtr<CC_SeqItem>,8> seq;
    Vec<CppOutReg *> reg_to_decl;
    CC_SeqItemBlock *sibling;

    mutable int n;
};

struct CC_SeqItemIf : CC_SeqItem {
    CC_SeqItemIf( CC_SeqItemBlock *parent );
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppRegConstraint &reg_constraints );
    virtual void assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints );
    virtual void get_seq_of_sub_blocks( Vec<std::pair<BoolOpSeq,CC_SeqItemBlock *> > &seq, const BoolOpSeq &cond );

    CC_SeqItemBlock seq[ 2 ];
    BoolOpSeq cond;
    int cur_seq;
};

struct CC_SeqItemExpr : CC_SeqItem {
    CC_SeqItemExpr( Expr expr, CC_SeqItemBlock *parent_block );
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppRegConstraint &reg_constraints );
    virtual void assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints );
    Vec<AutoPtr<CC_SeqItemBlock> > ext;
    Expr expr;
};

struct CC_SeqItemContinueOrBreak : CC_SeqItem {
    CC_SeqItemContinueOrBreak( bool cont, CC_SeqItemBlock *parent_block );
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppRegConstraint &reg_constraints );
    virtual void assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints );
    bool cont;
};

#endif // CC_SEQITEM_H

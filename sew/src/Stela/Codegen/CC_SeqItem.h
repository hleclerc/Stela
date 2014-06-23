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
    CC_SeqItemBlock *parent_block;
    CC_SeqItem *parent;
};

struct CC_SeqItemExpr : CC_SeqItem {
    CC_SeqItemExpr( Expr expr, CC_SeqItemBlock *parent );
    virtual ~CC_SeqItemExpr();
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppRegConstraint &reg_constraints );
    virtual void assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints );
    Expr expr;
};

struct CC_SeqItemBlock : CC_SeqItem {
    CC_SeqItemBlock();
    virtual ~CC_SeqItemBlock();
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppRegConstraint &reg_constraints );
    virtual void assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints );
    SplittedVec<AutoPtr<CC_SeqItem>,8> seq;
    Vec<CppOutReg *> reg_to_decl;
    CC_SeqItemBlock *sibling;

    mutable int n;
};

struct CC_SeqItemIf : CC_SeqItem {
    CC_SeqItemIf( CC_SeqItemBlock *parent );
    virtual ~CC_SeqItemIf();
    virtual void write( Codegen_C *cc );
    virtual void get_constraints( CppRegConstraint &reg_constraints );
    virtual void assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints );

    CC_SeqItemBlock seq[ 2 ];
    BoolOpSeq cond;
    int cur_seq;
};

#endif // CC_SEQITEM_H

#include "../Inst/CppRegConstraint.h"
#include "../Inst/Type.h"
#include "CC_SeqItem.h"
#include "Codegen_C.h"
#include "CppOutReg.h"

CC_SeqItem::CC_SeqItem( CC_SeqItem *parent, CC_SeqItemBlock *parent_block ) : parent_block( parent_block ), parent( parent ) {
}
CC_SeqItem::~CC_SeqItem() {
}



CC_SeqItemBlock::CC_SeqItemBlock() : CC_SeqItem( 0, 0 ), sibling( 0 ) {
}
CC_SeqItemBlock::~CC_SeqItemBlock() {
}
void CC_SeqItemBlock::write( Codegen_C *cc ) {
    std::map<Type *,Vec<CppOutReg *> > by_type;
    for( CppOutReg *r : reg_to_decl )
        by_type[ r->type ] << r;
    for( auto it : by_type ) {
        cc->on.write_beg();
        cc->write( it.first );
        for( int i = 0; i < it.second.size(); ++i )
            *cc->os << ( i ? ", R" : " R" ) << it.second[ i ]->num;
        cc->on.write_end( ";" );
    }

    for( int i = 0; i < seq.size(); ++i )
        seq[ i ]->write( cc );
}
void CC_SeqItemBlock::get_constraints( CppRegConstraint &reg_constraints ) {
    for( int n = 0; n < seq.size(); ++n )
        seq[ n ]->get_constraints( reg_constraints );
}
void CC_SeqItemBlock::assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints ) {
    for( int n = 0; n < seq.size(); ++n )
        seq[ n ]->assign_reg( cc, reg_constraints );
}



CC_SeqItemIf::CC_SeqItemIf( CC_SeqItemBlock *parent ) : CC_SeqItem( parent, parent ), cur_seq( 0 ) {
    for( int i = 0; i < 2; ++i ) {
        seq[ i ].parent_block = parent;
        seq[ i ].parent = this;
    }
    seq[ 0 ].sibling = &seq[ 1 ];
}
CC_SeqItemIf::~CC_SeqItemIf() {
}
void CC_SeqItemIf::write( Codegen_C *cc ) {
    if ( not cond.always( true ) ) {
        cond.write_to_stream( cc, cc->on.write_beg() << "if ( ", -1 );
        cc->on.write_end( " ) {" );
        cc->on.nsp += 4;
    }

    if ( not cond.always( false ) ) {
        seq[ 0 ].write( cc );
    }

    if ( not cond.always( true ) ) {
        if ( seq[ 1 ].seq.size() ) {
            cc->on.nsp -= 4;
            cc->on << "} else {";
            cc->on.nsp += 4;

            seq[ 1 ].write( cc );
        }

        cc->on.nsp -= 4;
        cc->on << "}";
    }
}

void CC_SeqItemIf::get_constraints( CppRegConstraint &reg_constraints ) {
    for( int i = 0; i < 2; ++i )
        seq[ i ].get_constraints( reg_constraints );
}

void CC_SeqItemIf::assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints ) {
    for( int i = 0; i < 2; ++i )
        seq[ i ].assign_reg( cc, reg_constraints );
}


CC_SeqItemExpr::CC_SeqItemExpr( Expr expr, CC_SeqItemBlock *parent ) : CC_SeqItem( parent, parent ), expr( expr ) {
}
CC_SeqItemExpr::~CC_SeqItemExpr() {
}
void CC_SeqItemExpr::write( Codegen_C *cc ) {
    CC_SeqItemBlock *b[ ext.size() ];
    for( int i = 0; i < ext.size(); ++i )
        b[ i ] = ext[ i ].ptr();
    expr->write( cc, b );
}
void CC_SeqItemExpr::get_constraints( CppRegConstraint &reg_constraints ) {
    expr->get_constraints( reg_constraints );
    for( AutoPtr<CC_SeqItemBlock> &b : ext )
        b->get_constraints( reg_constraints );
}
void CC_SeqItemExpr::assign_reg( Codegen_C *cc, CppRegConstraint &reg_constraints ) {
    if ( expr->need_a_register() ) {
        // constrained ?
        ++Inst::cur_op_id;
        expr->out_reg = reg_constraints.compulsory_reg( expr );
        // else,
        if ( not expr->out_reg )
            expr->out_reg = cc->new_out_reg( expr->type() );
        //
        expr->out_reg->provenance << parent_block;
    }
    for( AutoPtr<CC_SeqItemBlock> &b : ext )
        b->assign_reg( cc, reg_constraints );
}

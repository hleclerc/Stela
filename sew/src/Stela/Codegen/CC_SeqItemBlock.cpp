#include "../Inst/Type.h"

#include "CC_SeqItemContinueOrBreak.h"
#include "CC_SeqItemBlock.h"
#include "Codegen_C.h"
#include "CppOutReg.h"

CC_SeqItemBlock::CC_SeqItemBlock() : CC_SeqItem( 0, 0 ), sibling( 0 ) {
}

CC_SeqItemBlock::~CC_SeqItemBlock() {
}

void CC_SeqItemBlock::get_glo_cond_and_seq_of_sub_blocks( Vec<CC_SeqItemBlock *> &res, const BoolOpSeq &cond ) {
    glo_cond = cond;
    res << this;
    for( int i = 0; i < seq.size(); ++i )
        seq[ i ]->get_glo_cond_and_seq_of_sub_blocks( res, cond );
}

bool CC_SeqItemBlock::ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond ) {
    bool found = false;
    for( int i = 0; i < seq.size(); ++i ) {
        if ( found ) {
            if ( seq[ i ]->non_void() ) {
                if ( not cond.imply( not seq[ i ]->glo_cond ) )
                    return true;
                ++nb_evicted_blocks;
            }
        } else if ( seq[ i ] == ch )
            found = true;
    }
    return parent ? parent->ch_followed_by_something_to_execute( nb_evicted_blocks, this, cond ) : false;
}

bool CC_SeqItemBlock::visit( Visitor &v ) {
    for( int i = 0; i < seq.size(); ++i )
        if ( not seq[ i ]->visit( v ) )
            return false;
    return true;
}

bool CC_SeqItemBlock::non_void() {
    for( int i = 0; i < seq.size(); ++i )
        if ( seq[ i ]->non_void() )
            return true;
    return false;
}

void CC_SeqItemBlock::write( Codegen_C *cc ) {
    // type decl
    std::map<String,Vec<CppOutReg *> > by_type;
    for( CppOutReg *r : reg_to_decl )
        by_type[ cc->type_to_str( r->type ) ] << r;
    for( auto it : by_type ) {
        cc->on.write_beg() << it.first;
        for( int i = 0; i < it.second.size(); ++i )
            *cc->os << ( i ? ", R" : " R" ) << it.second[ i ]->num;
        cc->on.write_end( ";" );
    }

    // instructions
    for( int i = 0; i < seq.size(); ++i )
        seq[ i ]->write( cc );
}

bool CC_SeqItemBlock::contains_a_cont_or_break() {
    for( int n = 0; n < seq.size(); ++n )
        if ( dynamic_cast<CC_SeqItemContinueOrBreak *>( seq[ n ].ptr() ) )
            return true;
    return false;
}

void CC_SeqItemBlock::insert_before( CC_SeqItem *iter, CC_SeqItem *item ) {
    for( int o = 0; o < seq.size(); ++o ) {
        if ( seq[ o ] == iter ) {
            seq.insert( o, item );
            return;
        }
    }
}


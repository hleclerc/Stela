#include "CC_SeqItemContinueOrBreak.h"
#include "CC_SeqItemIf.h"
#include "Codegen_C.h"

CC_SeqItemIf::CC_SeqItemIf( CC_SeqItemBlock *parent ) : CC_SeqItem( parent, parent ), cur_seq( 0 ) {
    for( int i = 0; i < 2; ++i ) {
        seq[ i ].parent_block = parent;
        seq[ i ].parent = this;
    }
    seq[ 0 ].sibling = &seq[ 1 ];
}
void CC_SeqItemIf::get_glo_cond_and_seq_of_sub_blocks( Vec<CC_SeqItemBlock *> &res, const BoolOpSeq &cond ) {
    seq[ 0 ].get_glo_cond_and_seq_of_sub_blocks( res, cond and this->cond );
    seq[ 1 ].get_glo_cond_and_seq_of_sub_blocks( res, cond and not this->cond );
}

bool CC_SeqItemIf::visit( Visitor &v ) {
    for( int i = 0; i < 2; ++i )
        if ( not seq[ i ].visit( v ) )
            return false;
    return true;
}


bool CC_SeqItemIf::ch_followed_by_something_to_execute( int &nb_evicted_blocks, CC_SeqItem *ch, const BoolOpSeq &cond ) {
    return parent->ch_followed_by_something_to_execute( nb_evicted_blocks, this, cond );
}

bool CC_SeqItemIf::non_void() {
    return seq[ 0 ].non_void() or seq[ 1 ].non_void();
}
void CC_SeqItemIf::write( Codegen_C *cc ) {
    if ( seq[ 1 ].contains_a_cont_or_break() ) {
        // -> else block contains a break/continue => begin with the else block and write the if block without additionnal spaces
        ( not cond ).write_to_stream( cc, cc->on.write_beg() << "if ( ", -1 );
        cc->on.write_end( " ) {" );
        cc->on.nsp += 4;

        seq[ 1 ].write( cc );

        cc->on.nsp -= 4;
        cc->on << "}";

        seq[ 0 ].write( cc );
        return;
    }

    if ( not cond.always( true ) ) {
        cond.write_to_stream( cc, cc->on.write_beg() << "if ( ", -1 );
        cc->on.write_end( " ) {" );
        cc->on.nsp += 4;
    }

    if ( not cond.always( false ) )
        seq[ 0 ].write( cc );

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



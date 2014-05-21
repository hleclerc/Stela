#include "BoolOpSeq.h"

BoolOpSeq::BoolOpSeq( Expr expr, bool pos ) {
    or_seq.push_back()->push_back( Item{ expr, pos } );
}

BoolOpSeq::BoolOpSeq() {
}

void BoolOpSeq::write_to_stream( Stream &os ) const {
    for( int i = 0; i < or_seq.size(); ++i ) {
        if ( i )
            os << " or ";
        for( int j = 0; j < or_seq[ i ].size(); ++j ) {
            if ( j )
                os << " and ";
            if ( not or_seq[ i ][ j ].pos )
                os << "not ";
            os << or_seq[ i ][ j ].expr;
        }
    }
}

BoolOpSeq op_and( const BoolOpSeq &a, const BoolOpSeq &b ) {
    BoolOpSeq res;
    for( int i = 0; i < a.or_seq.size(); ++i ) {
        for( int j = 0; j < b.or_seq.size(); ++j ) {
            Vec<BoolOpSeq::Item> and_seq;
            for( int k = 0; k < a.or_seq[ i ].size(); ++k )
                and_seq << a.or_seq[ i ][ k ];
            for( int l = 0; l < b.or_seq[ j ].size(); ++l )
                and_seq << b.or_seq[ j ][ l ];
            res.or_seq << and_seq;
        }
    }
    return res;
}

BoolOpSeq op_or( const BoolOpSeq &a, const BoolOpSeq &b ) {
    BoolOpSeq res = a;
    for( int i = 0; i < b.or_seq.size(); ++i ) {
        for( int j = 0; ; ++j ) {
            if ( j == a.or_seq.size() ) {
                res.or_seq << b.or_seq[ i ];
                break;
            }
            if ( b.or_seq[ i ] == a.or_seq[ j ] )
                break;
        }
    }
    return res;
}

static void push_not_rec( BoolOpSeq &res, const BoolOpSeq &a, const Vec<int> &ind ) {
    if ( ind.size() == a.or_seq.size() ) {
        Vec<BoolOpSeq::Item> tmp;
        for( int i = 0; i < ind.size(); ++i )
            tmp << BoolOpSeq::Item{ a.or_seq[ i ][ ind[ i ] ].expr, not a.or_seq[ i ][ ind[ i ] ].pos };
        res.or_seq << tmp;
        return;
    }

    Vec<int> tmp = ind; tmp << 0;
    for( int i = 0; i < a.or_seq[ ind.size() ].size(); ++i ) {
        tmp.back() = i;
        push_not_rec( res, a, tmp );
    }
}


BoolOpSeq op_not( const BoolOpSeq &a ) {
    BoolOpSeq res;
    push_not_rec( res, a, Vec<int>() );
    return res;
}

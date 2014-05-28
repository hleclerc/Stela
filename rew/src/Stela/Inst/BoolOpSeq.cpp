#include "BoolOpSeq.h"
#include <algorithm>

BoolOpSeq::BoolOpSeq( Expr expr, bool pos ) {
    if ( expr->get_val( val_if_not_or_seq ) )
        val_if_not_or_seq ^= not pos;
    else
        or_seq.push_back()->push_back( Item{ expr, pos } );
}

BoolOpSeq::BoolOpSeq( bool pos ) : val_if_not_or_seq( pos ) {
}

void BoolOpSeq::write_to_stream( Stream &os ) const {
    if ( or_seq.size() ) {
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
    } else
        os << ( val_if_not_or_seq ? "true" : "false" );
}

bool BoolOpSeq::imply( const BoolOpSeq &b ) const {
    // val b knowing self
    if ( not b.or_seq.size() )
        return b.val_if_not_or_seq;
    // true =?> a or b: no
    if ( not or_seq.size() )
        return false;

    // ( a and b ) or c =?> b or c
    // a_0 or a_1 =?> b_0 or b1
    // ( b_0 or b_1 ) has to be true for a_0 alone, and a_1 alone
    for( const Vec<Item> &ai : or_seq ) { // test for a_0, then for a_1, ...
        for( int i = 0; ; ++i ) {
            if ( i == b.or_seq.size() )
                return false;
            if ( b.or_seq[ i ].subset_of( ai ) )
                break;
        }
    }
    return true;
}

Vec<BoolOpSeq::Item> BoolOpSeq::common_terms() const {
    if ( not or_seq.size() )
        return Vec<BoolOpSeq::Item>();
    Vec<BoolOpSeq::Item> res = or_seq[ 0 ];
    for( int i = 1; i < or_seq.size(); ++i )
        for( int j = 0; j < res.size(); ++j )
            if ( not or_seq[ i ].contains( res[ j ] ) )
                res.remove( j-- );
    return res;
}

int BoolOpSeq::nb_sub_conds() const {
    int res = 0;
    for( int i = 0; i < or_seq.size(); ++i )
        for( int j = 0; j < or_seq[ i ].size(); ++j )
            res += 2 - or_seq[ i ][ j ].pos;
    return res;
}

static bool eq_excepted( const Vec<BoolOpSeq::Item> &a, const Vec<BoolOpSeq::Item> &b, int &index ) {
    if ( a.size() != b.size() )
        return false;
    int n = 0;
    for( int i = 0; i < a.size(); ++i ) {
        if ( a[ i ].expr != b[ i ].expr )
            return false;
        if ( a[ i ].pos != b[ i ].pos ) {
            index = i;
            ++n;
        }
    }
    return n == 1;
}

BoolOpSeq &BoolOpSeq::simplify() {
    // simplify and_seqs
    if ( or_seq.size() ) {
        for( int i = 0; i < or_seq.size(); ++i )
            if ( not or_seq[ i ].size() )
                or_seq.remove( i-- );
        for( int i = 0; i < or_seq.size(); ++i )
            if ( simplify_and_seq( or_seq[ i ] ) )
                or_seq.remove( i-- );
        // no remaining possibility
        if ( not or_seq.size() )
            val_if_not_or_seq = false;
    }
    for( int i = 0; i < or_seq.size(); ++i ) {
        for( int j = i + 1; j < or_seq.size(); ++j ) {
            // c and c
            if ( or_seq[ i ] == or_seq[ j ] ) {
                or_seq.remove( j-- );
                continue;
            }
            // A and c or B and (!)c
            int index;
            if ( eq_excepted( or_seq[ i ], or_seq[ j ], index ) ) {
                // ( ... and c0 and ... ) or ( ... and not c0 and ... )
                if ( or_seq[ i ].size() > 1 ) {
                    or_seq[ i ].remove( index );
                    or_seq.remove( j-- );
                    continue;
                }
                // ...or c0 or not c0 or ...
                val_if_not_or_seq = true;
                or_seq.resize( 0 );
                return *this;
            }
            // ( C_i or C_j with C_i => C_j ) -> remove C_j
            if ( or_seq[ j ].subset_of( or_seq[ i ] ) ) {
                or_seq.remove( j-- );
                continue;
            }
            // ( C_i or C_j with C_j => C_i ) -> remove C_i
            if ( or_seq[ i ].subset_of( or_seq[ j ] ) ) {
                or_seq.remove( i-- );
                break;
            }
        }
    }

    std::sort( or_seq.begin(), or_seq.end() );

    return *this;
}

struct SortByExpr {
    bool operator()( const BoolOpSeq::Item &a, const BoolOpSeq::Item &b ) const {
        return a.expr.inst < b.expr.inst;
    }
};

bool BoolOpSeq::simplify_and_seq( Vec<Item> &and_seq ) {
    for( int i = 0; i < and_seq.size(); ++i ) {
        for( int j = i + 1; j < and_seq.size(); ++j ) {
            if ( and_seq[ i ].expr == and_seq[ j ].expr ) {
                if ( and_seq[ i ].pos != and_seq[ j ].pos )
                    return true; // c and not c
                // c and ... and c
                and_seq.remove( j-- );
            }
        }
    }
    std::sort( and_seq.begin(), and_seq.end(), SortByExpr() );
    return false;
}

BoolOpSeq operator&&( const BoolOpSeq &a, const BoolOpSeq &b ) {
    if ( not a.or_seq.size() )
        return a.val_if_not_or_seq ? b : BoolOpSeq( false );
    if ( not b.or_seq.size() )
        return b.val_if_not_or_seq ? a : BoolOpSeq( false );
    //
    BoolOpSeq res;
    for( int i = 0; i < a.or_seq.size(); ++i ) {
        for( int j = 0; j < b.or_seq.size(); ++j ) {
            Vec<BoolOpSeq::Item> &and_seq = *res.or_seq.push_back();
            for( int k = 0; k < a.or_seq[ i ].size(); ++k )
                and_seq << a.or_seq[ i ][ k ];
            for( int l = 0; l < b.or_seq[ j ].size(); ++l )
                and_seq << b.or_seq[ j ][ l ];
        }
    }
    return res.simplify();
}

BoolOpSeq operator||( const BoolOpSeq &a, const BoolOpSeq &b ) {
    if ( not a.or_seq.size() )
        return a.val_if_not_or_seq ? BoolOpSeq( true ) : b;
    if ( not b.or_seq.size() )
        return b.val_if_not_or_seq ? BoolOpSeq( true ) : a;
    //
    BoolOpSeq res;
    res.or_seq.append( a.or_seq );
    res.or_seq.append( b.or_seq );
    return res.simplify();
}

BoolOpSeq operator-( const BoolOpSeq &a, const BoolOpSeq &b ) {
    if ( b.or_seq.size() ) {
        BoolOpSeq res;
        for( const Vec<BoolOpSeq::Item> &bi : b.or_seq )
            for( const Vec<BoolOpSeq::Item> &ai : a.or_seq )
                res.or_seq << ai.without_vals_of( bi );
        return res.simplify();
    }
    return a;
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


BoolOpSeq operator!( const BoolOpSeq &a ) {
    if ( a.or_seq.size() ) {
        BoolOpSeq res;
        push_not_rec( res, a, Vec<int>() );
        return res;
    }
    return BoolOpSeq( not a.val_if_not_or_seq );
}


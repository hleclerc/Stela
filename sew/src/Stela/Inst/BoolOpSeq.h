#ifndef BOOLOPSEQ_H
#define BOOLOPSEQ_H

#include "Inst.h"
class Codegen_C;
class Expr;

/**
and( or( c0, not c1 ), or( ... ), ... )
*/
class BoolOpSeq {
public:
    struct Item {
        bool operator==( const Item &item ) const { return expr == item.expr and pos == item.pos; }
        bool operator!=( const Item &item ) const { return expr != item.expr or  pos != item.pos; }
        bool operator<( const Item &item ) const { if ( expr != item.expr ) return expr < item.expr; return pos < item.pos; }
        Item operator!() const { return { expr, not pos }; }
        void write_to_stream( Stream &os ) const { if ( not pos ) os << "not "; os << expr; }
        Expr expr;
        bool pos; ///< 0 -> not
    };

    BoolOpSeq( Expr expr, bool pos );
    BoolOpSeq( Item item );
    BoolOpSeq( False );
    BoolOpSeq( True );
    BoolOpSeq();

    void write_to_stream( Codegen_C *cc, Stream &os, int prec ) const;
    void write_to_stream( Stream &os ) const;
    BoolOpSeq &simplify();

    bool operator==( const BoolOpSeq &b ) const { return or_seq == b.or_seq and ( or_seq.size() or val_if_not_or_seq == b.val_if_not_or_seq ); }
    bool operator!=( const BoolOpSeq &b ) const { return not operator==( b ); }

    bool always( bool b ) const { return or_seq.size() == 0 and val_if_not_or_seq == b; }
    bool error();

    bool imply( const BoolOpSeq &b ) const; ///< val of b knowing this == true
    bool can_be_factorized_by( const Item &item ) const ;
    Vec<Item> common_terms() const;

    int nb_sub_conds() const;
    void get_out( Vec<Expr> &exprs, Vec<Vec<Bool> > &pos );

    struct InfMap {
        bool operator()( const BoolOpSeq &a, const BoolOpSeq &b ) const {
            if ( a.or_seq.size() != b.or_seq.size() )
                return a.or_seq.size() < b.or_seq.size();
            if ( a.or_seq.size() == 0 )
                return a.val_if_not_or_seq < b.val_if_not_or_seq;
            for( int i = 0; i < a.or_seq.size(); ++i ) {
                if ( a.or_seq[ i ].size() != b.or_seq[ i ].size() )
                    return a.or_seq[ i ].size() < b.or_seq[ i ].size();
                for( int j = 0; j < a.or_seq[ i ].size(); ++j ) {
                    if ( a.or_seq[ i ][ j ].expr != b.or_seq[ i ][ j ].expr )
                        return a.or_seq[ i ][ j ].expr < b.or_seq[ i ][ j ].expr;
                    if ( a.or_seq[ i ][ j ].pos != b.or_seq[ i ][ j ].pos )
                        return a.or_seq[ i ][ j ].pos < b.or_seq[ i ][ j ].pos;
                }
            }
            return false;
        }
    };

    Vec<Vec<Item> > or_seq; // ( c0 and c1 ) or ( c2 and c3 and ... ) or ...
    bool val_if_not_or_seq;
protected:
    bool simplify_and_seq( Vec<Item> &and_seq );
};

BoolOpSeq operator&&( const BoolOpSeq &a, const BoolOpSeq &b );
BoolOpSeq operator||( const BoolOpSeq &a, const BoolOpSeq &b );
BoolOpSeq operator- ( const BoolOpSeq &a, const BoolOpSeq &b );
BoolOpSeq operator! ( const BoolOpSeq &a );


#endif // BOOLOPSEQ_H

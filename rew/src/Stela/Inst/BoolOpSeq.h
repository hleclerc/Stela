#ifndef BOOLOPSEQ_H
#define BOOLOPSEQ_H

#include "Inst.h"
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
        void write_to_stream( Stream &os ) const { if ( not pos ) os << "not "; os << expr; }
        Expr expr;
        bool pos; ///< 0 -> not
    };

    BoolOpSeq( Expr expr, bool pos );
    BoolOpSeq( bool pos = true );

    void write_to_stream( Stream &os ) const;
    BoolOpSeq &simplify();

    bool operator==( const BoolOpSeq &b ) const { return or_seq == b.or_seq and ( or_seq.size() or val_if_not_or_seq == b.val_if_not_or_seq ); }
    bool operator!=( const BoolOpSeq &b ) const { return not operator==( b ); }

    bool always( bool b ) const { return or_seq.size() == 0 and val_if_not_or_seq == b; }

    bool imply( const BoolOpSeq &b ) const; ///< val of b knowing this == true
    Vec<Item> common_terms() const;

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

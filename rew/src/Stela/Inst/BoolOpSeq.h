#ifndef BOOLOPSEQ_H
#define BOOLOPSEQ_H

#include "Inst.h"

/**
and( or( c0, not c1 ), or( ... ), ... )
*/
class BoolOpSeq {
public:
    struct Item {
        bool operator==( const Item &item ) const { return expr == item.expr and pos == item.pos; }
        bool operator!=( const Item &item ) const { return expr != item.expr or  pos != item.pos; }
        Expr expr;
        bool pos; ///< 0 -> not
    };

    BoolOpSeq( Expr expr, bool pos );
    BoolOpSeq( bool pos = true );

    void write_to_stream( Stream &os ) const;
    BoolOpSeq &simplify();

    Vec<Vec<Item> > or_seq; // ( c0 and c1 ) or ( c2 and c3 and ... ) or ...
    bool val_if_not_or_seq;
protected:
    bool simplify_and_seq( Vec<Item> &and_seq );
};

BoolOpSeq op_and( const BoolOpSeq &a, const BoolOpSeq &b );
BoolOpSeq op_or ( const BoolOpSeq &a, const BoolOpSeq &b );
BoolOpSeq op_not( const BoolOpSeq &a );


#endif // BOOLOPSEQ_H

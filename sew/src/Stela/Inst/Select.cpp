#include "BoolOpSeq.h"
#include "Select.h"

/**
  ok, ko, conds (terms of the BoolOpSeq)
*/
struct Select : Inst {
    virtual void write_dot( Stream &os ) { os << "Select"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Select; }
    virtual Type *type() { return inp[ 0 ]->type(); }
    virtual Expr simplified( const BoolOpSeq &cond ) {
        BoolOpSeq bos = get_bos();
        if ( cond.imply( bos ) )
            return inp[ 0 ];
        if ( cond.imply( not bos ) )
            return inp[ 1 ];
        return this;
    }
    BoolOpSeq get_bos() {
        BoolOpSeq res;
        for( int i = 0, o = 2; i < pos.size(); ++i ) {
            Vec<BoolOpSeq::Item> *v = res.or_seq.push_back();
            for( int j = 0; j < pos[ i ].size(); ++j, ++o )
                v->push_back( BoolOpSeq::Item{ inp[ o ], pos[ i ][ j ] } );
        }
        return res;
    }
    Vec<Vec<Bool> > pos;
};

Expr select( const BoolOpSeq &cond, Expr ok, Expr ko ) {
    if ( cond.always( true  ) ) return ok;
    if ( cond.always( false ) ) return ko;
    if ( ok == ko ) return ok;

    Select *res = new Select;
    res->add_inp( ok );
    res->add_inp( ko );
    for( int i = 0; i < cond.or_seq.size(); ++i ) {
        Vec<Bool> *v = res->pos.push_back();
        for( int j = 0; j < cond.or_seq[ i ].size(); ++j ) {
            res->add_inp( cond.or_seq[ i ][ j ].expr );
            *v << cond.or_seq[ i ][ j ].pos;
        }
    }
    return res;
}


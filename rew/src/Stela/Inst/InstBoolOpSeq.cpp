#include "InstBoolOpSeq.h"
#include "Op.h"
#include "Ip.h"
#include <map>

/**
*/
class InstBoolOpSeq : public Inst {
public:
    struct Item {
        void write_to_stream( Stream &os ) const {
            if ( not pos )
                os << "!";
            os << index;
        }
        int  index;
        bool pos;
    };
    virtual void write_dot( Stream &os ) const {
        os << "bop[" << or_seq << "]";
    }
    virtual void write_to_stream( Stream &os, int prec ) const {
        if ( PREC_or_boolean <= prec )
            os << "(";
        int lprec = -1;
        for( int i = 0; i < or_seq.size(); ++i ) {
            if ( i )
                os << " or ";
            if ( or_seq.size() > 1 )
                lprec = PREC_or_boolean;
            for( int j = 0; j < or_seq[ i ].size(); ++j ) {
                if ( j )
                    os << " and ";
                if ( or_seq[ i ].size() > 1 )
                    lprec = PREC_and_boolean;
                if ( not or_seq[ i ][ j ].pos ) {
                    os << "not ";
                    lprec = PREC_not_boolean;
                }
                inp[ or_seq[ i ][ j ].index ]->write_to_stream( os, lprec );
            }
        }
        if ( PREC_or_boolean <= prec )
            os << ")";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        InstBoolOpSeq *res = new InstBoolOpSeq;
        res->or_seq = or_seq;
        return res;
    }
    virtual int size() const {
        return 1;
    }
    virtual BoolOpSeq get_BoolOpSeq() {
        BoolOpSeq res;
        for( int i = 0; i < or_seq.size(); ++i ) {
            auto *s = res.or_seq.push_back();
            for( int j = 0; j < or_seq[ i ].size(); ++j )
                *s << BoolOpSeq::Item{ inp[ or_seq[ i ][ j ].index ], or_seq[ i ][ j ].pos };
        }
        return res;
    }


    Vec<Vec<Item> > or_seq;
};

Expr inst_bool_op_seq( const BoolOpSeq &bos ) {
    if ( bos.or_seq.size() ) {
        std::map<Expr,int> ind;
        InstBoolOpSeq *res = new InstBoolOpSeq;
        for( int i = 0; i < bos.or_seq.size(); ++i ) {
            Vec<InstBoolOpSeq::Item> *v = res->or_seq.push_back();
            for( int j = 0; j < bos.or_seq[ i ].size(); ++j ) {
                Expr expr = bos.or_seq[ i ][ j ].expr;
                auto it = ind.insert( std::make_pair( expr, ind.size() ) );
                if ( it.second )
                    res->add_inp( expr );
                v->push_back( InstBoolOpSeq::Item{ it.first->second, bos.or_seq[ i ][ j ].pos } );
            }
        }
        return res;
    }
    return bos.val_if_not_or_seq ? ip->cst_true : ip->cst_false;
}


#include "../System/SameType.h"
#include "InstBoolOpSeq.h"
#include "InstInfo_C.h"
#include "BoolOpSeq.h"
#include "Type.h"
#include "Cst.h"
#include "Op.h"
#include "Ip.h"

/**
*/
template<class T>
class Op : public Inst {
public:
    Op( Type *tr, Type *ta, Type *tb = 0 ) : tr( tr ), ta( ta ), tb( tb ) {
    }
    virtual void write_dot( Stream &os ) const {
        os << T();
    }
    virtual void write_to_stream( Stream &os, int prec = -1 ) const {
        if ( T::is_oper ) {
            if ( T::prec <= prec )
                os << "(";
            if ( T::n == 1 ) {
                T().write_oper( os );
                os << " ";
                inp[ 0 ]->write_to_stream( os, T::prec );
            } else {
                inp[ 0 ]->write_to_stream( os, T::prec );
                os << " ";
                T().write_oper( os );
                os << " ";
                inp[ 1 ]->write_to_stream( os, T::prec );
            }
            if ( T::prec <= prec )
                os << ")";
        } else
            Inst::write_to_stream( os, prec );
    }
    virtual void update_out_type() {
        out_type_proposition( tr );
        if ( ta ) inp[ 0 ]->out_type_proposition( ta );
        if ( tb ) inp[ 1 ]->out_type_proposition( tb );
    }
    virtual int size() const {
        return tr->size();
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Op( tr, ta, tb );
    }

    virtual void write_to( Codegen_C *cc, int prec ) {
        if ( prec < 0 and par.size() <= 1 )
            return;
        if ( T::is_oper ) {
            if ( T::prec <= prec )
                *cc->os << "(";
            if ( T::n == 1 ) {
                T().write_oper( *cc->os );
                *cc->os << " " << cc->code( inp[ 0 ], T::prec );
            } else {
                *cc->os << cc->code( inp[ 0 ], T::prec ) << " ";
                T().write_oper( *cc->os );
                *cc->os << " " << cc->code( inp[ 1 ], T::prec );
            }
            if ( T::prec <= prec )
                *cc->os << ")";
        } else
            TODO;
    }

    Type *tr;
    Type *ta;
    Type *tb;
};


// ---------------------------------------------------------------------------------------
// binary specialisations
template<class OP>
Expr _op_simplication( Type *tr, Type *ta, Expr a, Type *tb, Expr b, OP ) {
    return 0;
}

// or
Expr _op_simplication( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_or_boolean ) {
    return inst_bool_op_seq( a->get_BoolOpSeq() or b->get_BoolOpSeq() );
}

// and
Expr _op_simplication( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_and_boolean ) {
    return inst_bool_op_seq( a->get_BoolOpSeq() and b->get_BoolOpSeq() );
}

//
template<class OP>
Expr _op( Type *tr, Type *ta, Expr a, Type *tb, Expr b, OP op ) {
    if ( Expr res = _op_simplication( tr, ta, a, tb, b, op ) )
        return res;

    Op<OP> *res = new Op<OP>( tr, ta, tb );
    res->add_inp( a );
    res->add_inp( b );
    return res;
}

// ---------------------------------------------------------------------------------------
// unary specialisations
template<class OP>
Expr _op_simplication( Type *tr, Type *ta, Expr a, OP ) {
    return 0;
}

// not_boolean
Expr _op_simplication( Type *tr, Type *ta, Expr a, Op_not_boolean ) {
    return inst_bool_op_seq( not a->get_BoolOpSeq() );
}

//
template<class OP>
Expr _op( Type *tr, Type *ta, Expr a, OP op ) {
    if ( Expr res = _op_simplication( tr, ta, a, op ) )
        return res;

    Op<OP> *res = new Op<OP>( tr, ta );
    res->add_inp( a );
    return res;
}


#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    Expr op( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_##NAME o ) { \
        return _op( tr, ta, a, tb, b, o ); \
    }
#include "DeclOp_Binary.h"
#undef DECL_OP

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    Expr op( Type *tr, Type *ta, Expr a, Op_##NAME o ) { \
        return _op( tr, ta, a, o ); \
    }
#include "DeclOp_Unary.h"
#undef DECL_OP

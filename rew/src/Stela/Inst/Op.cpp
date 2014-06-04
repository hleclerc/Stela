#include "../System/SameType.h"
#include "TypePromoteInt.h"
#include "InstBoolOpSeq.h"
#include "InstInfo_C.h"
#include "BoolOpSeq.h"
#include "FillAt.h"
#include "Slice.h"
#include "Type.h"
#include "Conv.h"
#include "Cst.h"
#include "Op.h"
#include "Ip.h"
#include <math.h>

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
        } else {
            if ( SameType<T,Op_mod>::res ) {
                if ( tr == &ip->type_FP32 or tr == &ip->type_FP64 ) {
                    cc->add_include( "<math.h>" );
                    *cc->os << "fmod";
                } else {
                    if ( T::prec <= prec ) *cc->os << "(";
                    *cc->os << cc->code( inp[ 0 ], T::prec ) << " % " << cc->code( inp[ 1 ], T::prec );
                    if ( T::prec <= prec ) *cc->os << ")";
                    return;
                }
            } else
                T().write_oper( *cc->os );

            *cc->os << "( " << cc->code( inp[ 0 ], T::prec );
            if ( T::n == 2 )
                *cc->os << ", " << cc->code( inp[ 1 ], T::prec );
            *cc->os << " )";
        }
    }

    virtual Expr _get_val( int len ) {
        if ( SameType<T,Op_add>::res )
            return slice( simplified( inp[ 0 ]->_get_val() ), tb, simplified( inp[ 1 ] ), len );
        return Inst::_get_val( len );
    }

    virtual void _set_val( Expr val, int len ) {
        if ( SameType<T,Op_add>::res )
            return inp[ 0 ]->_set_val(
                    fill_at( simplified( inp[ 0 ]->_get_val( inp[ 0 ]->size_ptd() ) ),
                             val, tb, simplified( inp[ 1 ] ) ),
                    len );
        return Inst::_set_val( val, len );
    }

    virtual bool is_a_pointer() const {
        return SameType<T,Op_add>::res and inp[ 0 ]->is_a_pointer();
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

// add
Expr _op_simplication( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_add ) {
    SI32 va, vb;
    if ( a->get_val( va, ta ) ) {
        if ( not va )
            return b;
    }
    if ( b->get_val( vb, tb ) ) {
        if ( not vb )
            return a;
    }
    return 0;
}

template<class T1,class T2>
bool my_xor( T1 a, T2 b ) {
    return bool( a ) ^ bool( b );
}

template<class T1,class T2> typename TypePromoteInt<T1,T2>::T my_mod( T1 a, T2 b ) { return a % b; }
template<class T1> FP32 my_mod( T1 a, FP32 b ) { return fmod( a, b ); }
template<class T1> FP64 my_mod( T1 a, FP64 b ) { return fmod( a, b ); }
template<class T2> FP32 my_mod( FP32 a, T2 b ) { return fmod( a, b ); }
template<class T2> FP64 my_mod( FP64 a, T2 b ) { return fmod( a, b ); }
FP32 my_mod( FP32 a, FP32 b ) { return fmod( a, b ); }
FP64 my_mod( FP32 a, FP64 b ) { return fmod( a, b ); }
FP64 my_mod( FP64 a, FP32 b ) { return fmod( a, b ); }
FP64 my_mod( FP64 a, FP64 b ) { return fmod( a, b ); }


#define DECL_BT( TR ) Expr _conv( Type *tr, const TR &v ) { return conv( tr, &ip->type_##TR, cst( v ) ); }
#include "DeclArytTypes.h"
#undef DECL_BT

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    template<class TA,class TB> \
    Expr _op_cst( Type *tr, TA *da, TB *db, Op_##NAME ) { return _conv( tr, *da GEN *db ); }
#include "DeclOp_Binary_Oper.h"
#undef DECL_OP

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    template<class TA,class TB> \
    Expr _op_cst( Type *tr, TA *da, TB *db, Op_##NAME ) { return _conv( tr, GEN( *da, *db ) ); }
#include "DeclOp_Binary_Func.h"
#undef DECL_OP


template<class TA,class OP>
Expr _op_cst( Type *tr, TA *da, Type *tb, PI8 *db, OP op ) {
    #define DECL_BT( T ) if ( tb == &ip->type_##T ) return _op_cst( tr, da, reinterpret_cast<T *>( db ), op );
    #include "DeclArytTypes.h"
    #undef DECL_BT
    return Expr();
}

//
template<class OP>
Expr _op( Type *tr, Type *ta, Expr a, Type *tb, Expr b, OP op ) {
    // cst op cst ?
    PI8 da[ ( ta->size() + 7 ) / 8 ];
    PI8 db[ ( ta->size() + 7 ) / 8 ];
    if ( a->get_val( da, ta->size() ) and b->get_val( db, tb->size() ) ) {
        #define DECL_BT( T ) if ( ta == &ip->type_##T ) if ( Expr res = _op_cst( tr, reinterpret_cast<T *>( da ), tb, db, op ) ) return res;
        #include "DeclArytTypes.h"
        #undef DECL_BT
    }


    // specialized simplifications
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

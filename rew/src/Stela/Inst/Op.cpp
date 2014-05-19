#include "../System/SameType.h"
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
    virtual Type *out_type_proposition( Codegen_C *cc ) const {
        return tr;
    }
    virtual Type *inp_type_proposition( Codegen_C *cc, int ninp ) const {
        return ninp ? tb : ta;
    }
    virtual int size() const {
        return tr->size();
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Op( tr, ta, tb );
    }


    // checked_if
    template<class TT>
    int _checked_if( Expr cond, TT ) {
        return Inst::checked_if( cond );
    }
    int _checked_if( Expr cond, Op_and_boolean ) {
        int c0 = inp[ 0 ]->checked_if( cond );
        int c1 = inp[ 1 ]->checked_if( cond );
        if ( c0 == 1 and c1 == 1 ) // both are ok
            return 1;
        if ( c0 == -1 or c1 == -1 ) // at least one is false
            return -1;
        return 0;
    }
    int _checked_if( Expr cond, Op_or_boolean ) {
        int c0 = inp[ 0 ]->checked_if( cond );
        int c1 = inp[ 1 ]->checked_if( cond );
        if ( c0 == 1 or c1 == 1 ) // at least one is ok
            return 1;
        if ( c0 == -1 and c1 == -1 ) // both are false
            return -1;
        return 0;
    }
    int _checked_if( Expr cond, Op_not_boolean ) {
        if ( int c0 = inp[ 0 ]->checked_if( cond ) )
            return -c0;
        return 0;
    }

    virtual int checked_if( Expr cond ) {
        return _checked_if( cond, T() );
    }

    // rtrue if
    template<class TT>
    int _allow_to_check( Expr val, TT ) {
        return 0;
    }
    int _allow_to_check( Expr val, Op_and_boolean ) {
        // we know that inp[ 0 ] and inp[ 1 ] are true
        int c0 = inp[ 0 ]->allow_to_check( val );
        int c1 = inp[ 1 ]->allow_to_check( val );
        if ( c0 == 1 or c1 == 1 )
            return 1;
        if ( c0 == -1 and c1 == -1 )
            return -1;
        return 0;
    }
    int _allow_to_check( Expr val, Op_or_boolean ) {
        // we know that inp[ 0 ] or inp[ 1 ] are true
        int c0 = inp[ 0 ]->allow_to_check( val );
        int c1 = inp[ 1 ]->allow_to_check( val );
        if ( c0 == 1 and c1 == 1 )
            return 1;
        if ( c0 == -1 and c1 == -1 )
            return -1;
        return 0;
    }
    int _allow_to_check( Expr val, Op_not_boolean ) {
        if ( int c0 = inp[ 0 ]->allow_to_check( val ) )
            return -c0;
        return 0;
    }

    virtual int allow_to_check( Expr val ) {
        if ( int trial = Inst::allow_to_check( val ) )
            return trial;
        return _allow_to_check( val, T() );
    }

    virtual void _get_sub_cond_or( Vec<std::pair<Expr,bool> > &sc, bool pos ) {
        if ( SameType<T,Op_or_boolean>::res ) {
            inp[ 0 ]->_get_sub_cond_or( sc, pos );
            inp[ 1 ]->_get_sub_cond_or( sc, pos );
            return;
        }
        if ( SameType<T,Op_not_boolean>::res )
            return inp[ 0 ]->_get_sub_cond_or( sc, not pos );
        sc << std::make_pair( this, pos );
    }

    virtual void _get_sub_cond_and( Vec<std::pair<Expr,bool> > &sc, bool pos ) {
        if ( SameType<T,Op_and_boolean>::res ) {
            inp[ 0 ]->_get_sub_cond_and( sc, pos );
            inp[ 1 ]->_get_sub_cond_and( sc, pos );
            return;
        }
        if ( SameType<T,Op_not_boolean>::res )
            return inp[ 0 ]->_get_sub_cond_and( sc, not pos );
        sc << std::make_pair( this, pos );
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
static Expr make_or( Vec<std::pair<Expr,bool> > &sc, int beg, int end ) {
    if ( beg == end )
        return ip->cst_true;
    if ( beg + 1 == end )
        return sc[ beg ].second ? sc[ beg ].first : op( &ip->type_Bool, &ip->type_Bool, sc[ beg ].first, Op_not_boolean() );
    return op( &ip->type_Bool, &ip->type_Bool, make_or( sc, beg, ( beg + end ) / 2 ), &ip->type_Bool, make_or( sc, ( beg + end ) / 2, end ), Op_or_boolean() );
}

Expr _op_simplication( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_or_boolean ) {
    Bool val;
    if ( a == b )
        return a;
    if ( a->get_val( val ) )
        return val ? a : b;
    if ( b->get_val( val ) )
        return val ? b : a;

    Vec<std::pair<Expr,bool> > sc;
    a->_get_sub_cond_or( sc, true );
    int sa = sc.size();
    b->_get_sub_cond_or( sc, true );
    int sb = sc.size();
    for( int i = 0; i < sa; ++i ) {
        for( int j = sa; j < sc.size(); ++j ) {
            if ( sc[ i ].first == sc[ j ].first ) {
                if ( sc[ i ].second == sc[ j ].second )
                    sc.remove( j-- );
                else {
                    sc.remove( j );
                    sc.remove( i-- );
                    --sa;
                    break;
                }
            }
        }
    }
    if ( sb != sc.size() )
        return make_or( sc, 0, sc.size() );

    return 0;
}

// and
static Expr make_and( Vec<std::pair<Expr,bool> > &sc, int beg, int end ) {
    if ( beg == end )
        ERROR( "weirdos");
    if ( beg + 1 == end )
        return sc[ beg ].second ? sc[ beg ].first : op( &ip->type_Bool, &ip->type_Bool, sc[ beg ].first, Op_not_boolean() );
    return op( &ip->type_Bool, &ip->type_Bool, make_and( sc, beg, ( beg + end ) / 2 ), &ip->type_Bool, make_and( sc, ( beg + end ) / 2, end ), Op_and_boolean() );
}

Expr _op_simplication( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_and_boolean ) {
    Bool val;
    if ( a == b )
        return a;
    if ( a->get_val( val ) )
        return val ? b : a;
    if ( b->get_val( val ) )
        return val ? a : b;
    if ( int res = a->checked_if( b ) )
        return res > 0 ? b : ip->cst_false; // knowing b is enough to know the result; if b ==> not a, -> false
    if ( int res = b->checked_if( a ) )
        return res > 0 ? a : ip->cst_false; // knowing b is enough to know the result; if a ==> not v, -> false

    Vec<std::pair<Expr,bool> > sc;
    a->_get_sub_cond_and( sc, true );
    int sa = sc.size();
    b->_get_sub_cond_and( sc, true );
    int sb = sc.size();
    for( int i = 0; i < sa; ++i ) {
        for( int j = sa; j < sc.size(); ++j ) {
            if ( sc[ i ].first == sc[ j ].first ) {
                if ( sc[ i ].second == sc[ j ].second )
                    sc.remove( j-- );
                else
                    return ip->cst_false;
            }
        }
    }
    if ( sb != sc.size() )
        return make_and( sc, 0, sc.size() );


    return 0;
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
    Bool val;
    if ( a->get_val( val ) )
        return val ? ip->cst_false : ip->cst_true;
    return 0;
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


#define DECL_OP( NAME, GEN, OPER, BOOL ) \
    Expr op( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_##NAME o ) { \
        return _op( tr, ta, a, tb, b, o ); \
    }
#include "DeclOp_Binary.h"
#undef DECL_OP

#define DECL_OP( NAME, GEN, OPER, BOOL ) \
    Expr op( Type *tr, Type *ta, Expr a, Op_##NAME o ) { \
        return _op( tr, ta, a, o ); \
    }
#include "DeclOp_Unary.h"
#undef DECL_OP

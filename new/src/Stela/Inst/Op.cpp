#include "../CodeGen/InstInfo_C.h"
#include "Type.h"
#include "Op.h"

template<class T>
struct OpCout {};

#define DECL_OP( NAME, GEN, OPER, BOOL ) \
    template<> struct OpCout<Op_##NAME> { \
        enum { oper = OPER }; \
        void write_to_stream( Stream &os ) const { os << GEN; } \
    };
#include "DeclOp_Binary.h"
#undef DECL_OP

/**
*/
template<class T>
class Op : public Inst {
public:
    Op( Type *tr, Type *ta, Type *tb ) : tr( tr ), ta( ta ), tb( tb ) {
    }
    virtual void write_dot( Stream &os ) const {
        os << T();
    }
    virtual int size() const {
        return tr->size();
    }
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const {
        return new Op( tr, ta, tb );
    }
    virtual void write_1l_to( CodeGen_C *cc ) const {
        if ( OpCout<T>::oper )
            *cc->os
                << cc->code( inp[ 0 ] ) << ' '
                << OpCout<T>() << ' '
                << cc->code( inp[ 1 ] );
        else
            *cc->os
                << OpCout<T>() << "( "
                << cc->code( inp[ 0 ] ) << ", "
                << cc->code( inp[ 1 ] ) << " )";
    }
    virtual Type *out_type_proposition( CodeGen_C *cc ) const {
        return tr;
    }
    virtual Type *inp_type_proposition( CodeGen_C *cc, int ninp ) const {
        return ninp ? tb : ta;
    }

    // true if
    template<class TT>
    bool _true_if( const ConstPtr<Inst> &cond, TT ) const {
        return 0;
    }
    bool _true_if( const ConstPtr<Inst> &cond, Op_and ) const {
        return inp[ 0 ]->true_if( cond ) and inp[ 1 ]->true_if( cond );
    }
    bool _true_if( const ConstPtr<Inst> &cond, Op_mul ) const {
        return inp[ 0 ]->true_if( cond ) and inp[ 1 ]->true_if( cond );
    }
    bool _true_if( const ConstPtr<Inst> &cond, Op_or ) const {
        return inp[ 0 ]->true_if( cond ) or inp[ 1 ]->true_if( cond );
    }

    virtual bool true_if( const ConstPtr<Inst> &cond ) const {
        if ( Inst::true_if( cond ) )
            return true;
        return _true_if( cond, T() );
    }

    // rtrue if
    template<class TT>
    bool _rtrue_if( const ConstPtr<Inst> &val, TT ) const {
        return 0;
    }
    bool _rtrue_if( const ConstPtr<Inst> &val, Op_and ) const {
        return val->true_if( inp[ 0 ] ) or val->true_if( inp[ 1 ] );
    }
    bool _rtrue_if( const ConstPtr<Inst> &val, Op_mul ) const {
        return val->true_if( inp[ 0 ] ) or val->true_if( inp[ 1 ] );
    }
    bool _rtrue_if( const ConstPtr<Inst> &val, Op_or ) const {
        return val->true_if( inp[ 0 ] ) and val->true_if( inp[ 1 ] );
    }

    virtual bool rtrue_if( const ConstPtr<Inst> &val ) const {
        if ( Inst::rtrue_if( val ) )
            return true;
        return _rtrue_if( val, T() );
    }

    virtual bool is_and() const {
        return true;
    }



    Type *tr;
    Type *ta;
    Type *tb;
};

// ---------------------------------------------------------------------------------------
// specialisations
template<class OP>
Ptr<Inst> _op_simplication( Type *tr, Type *ta, Ptr<Inst> a, Type *tb, Ptr<Inst> b, OP ) {
    return 0;
}

// or
Ptr<Inst> _op_simplication( Type *tr, Type *ta, Ptr<Inst> a, Type *tb, Ptr<Inst> b, Op_or ) {
    Bool val;
    if ( a->get_val( val ) )
        return val ? a : b;
    if ( b->get_val( val ) )
        return val ? b : a;
    return 0;
}

// and
Ptr<Inst> _op_simplication( Type *tr, Type *ta, Ptr<Inst> a, Type *tb, Ptr<Inst> b, Op_and ) {
    Bool val;
    if ( a->get_val( val ) )
        return val ? b : a;
    if ( b->get_val( val ) )
        return val ? a : b;
    if ( a->true_if( b ) )
        return b; // knowing b is enough to known the result
    if ( b->true_if( a ) )
        return a; // knowing b is enough to known the result
    return 0;
}

//
template<class OP>
Ptr<Inst> _op( Type *tr, Type *ta, Ptr<Inst> a, Type *tb, Ptr<Inst> b, OP op ) {
    if ( Ptr<Inst> res = _op_simplication( tr, ta, a, tb, b, op ) )
        return res;

    Op<OP> *res = new Op<OP>( tr, ta, tb );
    res->add_inp( a );
    res->add_inp( b );
    return res;
}

#define DECL_OP( NAME, GEN, OPER, BOOL ) \
    Ptr<Inst> op( Type *tr, Type *ta, Ptr<Inst> a, Type *tb, Ptr<Inst> b, Op_##NAME o ) { \
        return _op( tr, ta, a, tb, b, o ); \
    }
#include "DeclOp_Binary.h"
#undef DECL_OP

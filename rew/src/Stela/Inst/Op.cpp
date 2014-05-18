#include "Type.h"
#include "Op.h"

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
    virtual Type *out_type_proposition( CodeGen_C *cc ) const {
        return tr;
    }
    virtual Type *inp_type_proposition( CodeGen_C *cc, int ninp ) const {
        return ninp ? tb : ta;
    }
    virtual int size() const {
        return tr->size();
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Op( tr, ta, tb );
    }


    // true if
    template<class TT>
    Expr _val_if( Expr cond, TT ) {
        return this;
    }
    Expr _val_if( Expr cond, Op_and_boolean ) {
        TODO;
        return inp[ 0 ];
        // return inp[ 0 ]->ok_if( cond ) and inp[ 1 ]->ok_if( cond );
    }
    Expr _val_if( Expr cond, Op_or_boolean ) {
        TODO;
        return inp[ 0 ];
        // return inp[ 0 ]->ok_if( cond ) or inp[ 1 ]->ok_if( cond );
    }

    virtual Expr val_if( Expr cond ) {
        Expr trial = Inst::val_if( cond );
        if ( trial.ptr() != this )
            return trial;
        return _val_if( cond, T() );
    }

    // rtrue if
    template<class TT>
    Expr _rval_if( Expr val, TT ) {
        return 0;
    }
    Expr _rval_if( Expr val, Op_and_boolean ) {
        TODO;
        return inp[ 0 ];
        // return val->ok_if( inp[ 0 ] ) or val->ok_if( inp[ 1 ] );
    }
    Expr _rval_if( Expr val, Op_or_boolean ) {
        TODO;
        return inp[ 0 ];
        // return val->ok_if( inp[ 0 ] ) and val->ok_if( inp[ 1 ] );
    }

    virtual Expr rval_if( Expr val ) {
        Expr trial = Inst::rval_if( val );
        if ( trial != val )
            return trial;
        return _rval_if( val, T() );
    }

    Type *tr;
    Type *ta;
    Type *tb;
};

// ---------------------------------------------------------------------------------------
// specialisations
template<class OP>
Expr _op_simplication( Type *tr, Type *ta, Expr a, Type *tb, Expr b, OP ) {
    return 0;
}

// or
Expr _op_simplication( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_or_boolean ) {
    Bool val;
    if ( a->get_val( val ) )
        return val ? a : b;
    if ( b->get_val( val ) )
        return val ? b : a;
    return 0;
}

// and
Expr _op_simplication( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_and_boolean ) {
    Bool val;
    if ( a->get_val( val ) )
        return val ? b : a;
    if ( b->get_val( val ) )
        return val ? a : b;
    if ( a->ok_if( b ) )
        return b; // knowing b is enough to known the result
    if ( b->ok_if( a ) )
        return a; // knowing b is enough to known the result
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

#define DECL_OP( NAME, GEN, OPER, BOOL ) \
    Expr op( Type *tr, Type *ta, Expr a, Type *tb, Expr b, Op_##NAME o ) { \
        return _op( tr, ta, a, tb, b, o ); \
    }
#include "DeclOp_Binary.h"
#undef DECL_OP

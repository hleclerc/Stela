#include "InstVisitor.h"
#include "OpStructs.h"
#include "Inst_.h"
#include "Cst.h"
#include "Op.h"

/**
*/
template<class TOP>
class Op : public Inst_<1,TOP::nb_ch> {
public:
    virtual int size_in_bits( int nout ) const { return bt->size_in_bits(); }
    virtual void write_to_stream( Stream &os ) const {
        os << TOP::name() << "(";
        for( int i = 0; i < TOP::nb_ch; ++i ) {
            if ( i )
                os << ",";
            os << this->inp_expr( i );
        }
        os << ")";
    }
    virtual void apply( InstVisitor &visitor ) const;
    virtual int inst_id() const { return Inst::Id_Op + TOP::op_id; }
    virtual bool equal( const Inst *b ) const {
        return Inst::equal( b ) and bt == static_cast<const Op *>( b )->bt;
    }
    virtual const BaseType *out_bt( int n ) const {
        return bt;
    }
    virtual const PI8 *cst_data_ValAt( int nout, int off ) const {
        if ( TOP::op_id == ID_add ) {
            Expr a = this->inp_expr( 0 );
            Expr b = this->inp_expr( 1 );
            if ( bt == bt_SI64 or bt == bt_PI64 ) {
                SI64 nff = 0;
                // ptr + off
                if ( b.basic_conv( nff ) )
                    if ( const PI8 *da = a.inst->cst_data_ValAt( a.nout, off + nff ) )
                        return da;
            }
            if ( bt == bt_SI32 or bt == bt_PI32 ) {
                SI32 nff = 0;
                // ptr + off
                if ( b.basic_conv( nff ) )
                    if ( const PI8 *da = a.inst->cst_data_ValAt( a.nout, off + nff ) )
                        return da;
            }
            return 0;
        }
        return 0;
    }

    const BaseType *bt;
};

// simplification rules
#define DECL_OP( OP ) \
    static Expr _simplify_cst( Op_##OP, const BaseType *bt, const PI8 *da, const PI8 *db ) { \
        if ( da and db ) { \
            Vec<PI8> res( Size(), bt->size_in_bytes() ); \
            bt->OP( res.ptr(), da, db ); \
            return cst( res ); \
        } \
        return Expr(); \
    }
#include "DeclOpBinary.h"
#undef DECL_OP

#define DECL_OP( OP ) \
    static Expr _simplify_cst( Op_##OP, const BaseType *bt, const PI8 *da ) { \
        if ( da ) { \
            Vec<PI8> res( Size(), bt->size_in_bytes() ); \
            bt->OP( res.ptr(), da ); \
            return cst( res ); \
        } \
        return Expr(); \
    }
#include "DeclOpUnary.h"
#undef DECL_OP


template<class TOP>
static Expr _simplify( TOP op, const BaseType *bt, const PI8 *da, const PI8 *db ) {
    if ( Expr res = _simplify_cst( op, bt, da, db ) )
        return res;
    return Expr();
}

template<class TOP>
static Expr _simplify( TOP op, const BaseType *bt, const PI8 *da ) {
    if ( Expr res = _simplify_cst( op, bt, da ) )
        return res;
    return Expr();
}

// factory
template<class TOP>
static Expr _op( TOP top, const BaseType *bt, Expr a, Expr b ) {
    ASSERT( bt->size_in_bits() <= a.size_in_bits(), "wrong size" );
    ASSERT( bt->size_in_bits() <= b.size_in_bits(), "wrong size" );

    // known values ?
    const PI8 *da = a.cst_data();
    const PI8 *db = b.cst_data();
    if ( Expr res = _simplify( top, bt, da, db ) )
        return res;

    // else, create a new inst
    Op<TOP> *res = new Op<TOP>;
    res->inp_repl( 0, a );
    res->inp_repl( 1, b );
    res->bt = bt;
    return Expr( Inst::factorized( res ), 0 );
}

template<class TOP>
static Expr _op( TOP top, const BaseType *bt, Expr a ) {
    ASSERT( bt->size_in_bits() <= a.size_in_bits(), "wrong size" );

    // known values ?
    const PI8 *da = a.cst_data();
    if ( Expr res = _simplify( top, bt, da ) )
        return res;

    // else, create a new inst
    Op<TOP> *res = new Op<TOP>;
    res->inp_repl( 0, a );
    res->bt = bt;
    return Expr( Inst::factorized( res ), 0 );
}

// apply
#define DECL_OP( OP ) template<> void Op<Op_##OP>::apply( InstVisitor &visitor ) const { visitor.OP( *this, bt ); }
#include "DeclOp.h"
#undef DECL_OP

// functions
#define DECL_OP( OP ) Expr OP( const BaseType *bt, Expr a, Expr b ) { return _op( Op_##OP(), bt, a, b ); }
#include "DeclOpBinary.h"
#undef DECL_OP

#define DECL_OP( OP ) Expr OP( const BaseType *bt, Expr a ) { return _op( Op_##OP(), bt, a ); }
#include "DeclOpUnary.h"
#undef DECL_OP

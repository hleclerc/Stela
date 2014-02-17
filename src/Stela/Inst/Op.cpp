#include "InstVisitor.h"
#include "OpStructs.h"
#include "Inst_.h"
#include "ValAt.h"
#include "Cst.h"
#include "Op.h"

/**
*/
template<class TOP>
class Op : public Inst_<1,TOP::nb_ch> {
public:
    virtual int size_in_bits( int nout ) const { return bt->size_in_bits(); }
    virtual void write_dot( Stream &os ) const { os << TOP::name(); }
    virtual void apply( InstVisitor &visitor ) const;
    virtual int inst_id() const { return Inst::Id_Op + TOP::op_id; }
    virtual bool equal( const Inst *b ) const {
        return Inst::equal( b ) and bt == static_cast<const Op *>( b )->bt;
    }
    virtual const BaseType *out_bt( int n ) const {
        return bt;
    }
    virtual const PI8 *vat_data( int nout, int beg, int end ) const {
        if ( TOP::op_id == int( ID_add ) ) {
            Expr a = this->inp_expr( 0 );
            Expr b = this->inp_expr( 1 );
            if ( bt == bt_SI64 or bt == bt_PI64 ) {
                SI64 nff = 0;
                if ( b.get_val( nff ) )
                    if ( const PI8 *da = a.inst->vat_data( a.nout, beg + 8 * nff, end + 8 * nff ) )
                        return da;
            }
            if ( bt == bt_SI32 or bt == bt_PI32 ) {
                SI32 nff = 0;
                if ( b.get_val( nff ) )
                    if ( const PI8 *da = a.inst->vat_data( a.nout, beg + 8 * nff, end + 8 * nff ) )
                        return da;
            }
        }
        return 0;
    }
    virtual Expr _smp_val_at( int nout, int beg, int end ) {
        if ( Expr res = Inst::_smp_val_at( nout, beg, end ) )
            return res;
        if ( TOP::op_id == int( ID_add ) ) {
            Expr a = this->inp_expr( 0 );
            Expr b = this->inp_expr( 1 );
            if ( bt == bt_SI64 or bt == bt_PI64 ) {
                SI64 nff = 0;
                if ( b.get_val( nff ) )
                    return val_at( a, beg + 8 * nff, end + 8 * nff );
            }
            if ( bt == bt_SI32 or bt == bt_PI32 ) {
                SI32 nff = 0;
                if ( b.get_val( nff ) )
                    return val_at( a, beg + 8 * nff, end + 8 * nff );
            }

        }
        return Expr();
    }


    const BaseType *bt;
};

// simplification rules
#define DECL_IR_TOK( OP ) \
    static Expr _simplify_cst( Op_##OP, const BaseType *bt, const PI8 *da, const PI8 *db ) { \
        if ( da and db ) { \
            Vec<PI8> res( Size(), bt->size_in_bytes() ); \
            bt->op_##OP( res.ptr(), da, db ); \
            return cst( res ); \
        } \
        return Expr(); \
    }
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( OP ) \
    static Expr _simplify_cst( Op_##OP, const BaseType *bt, const PI8 *da ) { \
        if ( da ) { \
            Vec<PI8> res( Size(), bt->size_in_bytes() ); \
            bt->op_##OP( res.ptr(), da ); \
            return cst( res ); \
        } \
        return Expr(); \
    }
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK


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
    ASSERT( bt->size_in_bits() <= a.size_in_bits(), "wrong size (base type do not correspond to arg data)" );
    ASSERT( bt->size_in_bits() <= b.size_in_bits(), "wrong size (base type do not correspond to arg data)" );

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
#define DECL_IR_TOK( OP ) template<> void Op<Op_##OP>::apply( InstVisitor &visitor ) const { visitor.op_##OP( *this, bt ); }
#include "../Ir/Decl_Operations.h"
#undef DECL_IR_TOK

// functions
#define DECL_IR_TOK( OP ) Expr op_##OP( const BaseType *bt, Expr a, Expr b ) { return _op( Op_##OP(), bt, a, b ); }
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( OP ) Expr op_##OP( const BaseType *bt, Expr a ) { return _op( Op_##OP(), bt, a ); }
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( OP ) Expr op( const BaseType *bt, Expr a, Expr b, Op_##OP ) { return op_##OP( bt, a, b ); }
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( OP ) Expr op( const BaseType *bt, Expr a, Op_##OP ) { return op_##OP( bt, a ); }
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

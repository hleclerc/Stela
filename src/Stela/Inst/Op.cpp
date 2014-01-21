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
    virtual int size_in_bits( int nout ) const { return bt->size_in_bytes(); }
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
    virtual int inst_id() const { return 100 + TOP::op_id; }
    virtual bool equal( const Inst *b ) const {
        return Inst::equal( b ) and bt == static_cast<const Op *>( b )->bt;
    }

    const BaseType *bt;
};

// simplification rules
template<class TOP>
static Expr _simplify( TOP, const BaseType *bt, const PI8 *da, const PI8 *db ) {
    return Expr();
}

static Expr _simplify( Op_add, const BaseType *bt, const PI8 *da, const PI8 *db ) {
    if ( da and db ) {
        Vec<PI8> res( Size(), bt->size_in_bytes() );
        bt->add( res.ptr(), da, db );
        return cst( res );
    }
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
    return Expr( Inst::factorized( res ), 0 );
}

template<class TOP>
static Expr _op( TOP, const BaseType *bt, Expr a ) {
    Op<TOP> *res = new Op<TOP>;
    res->inp_repl( 0, a );
    return Expr( Inst::factorized( res ), 0 );
}

#define DECL_OP( OP ) template<> void Op<Op_##OP>::apply( InstVisitor &visitor ) const { visitor.OP( *this, bt ); }
#include "DeclOpBinary.h"
#undef DECL_OP

#define DECL_OP( OP ) Expr OP( const BaseType *bt, Expr a, Expr b ) { return _op( Op_##OP(), bt, a, b ); }
#include "DeclOpBinary.h"
#undef DECL_OP

#define DECL_OP( OP ) Expr OP( const BaseType *bt, Expr a ) { return _op( Op_##OP(), bt, a ); }
#include "DeclOpUnary.h"
#undef DECL_OP

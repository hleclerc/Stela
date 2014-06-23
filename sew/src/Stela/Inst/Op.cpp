#include "../Codegen/Codegen_C.h"
//#include "../Codegen/CppOutReg.h"
#include "../System/SameType.h"
#include "../System/Math.h"
#include "ReplBits.h"
#include "Slice.h"
#include "Type.h"
#include "Conv.h"
#include "Cst.h"
#include "Op.h"
#include "Ip.h"

template<class TO>
static Type *type_promote( Type *a, Type *b, TO ) {
    if ( TO::b )
        return ip->type_Bool;
    if ( a == b or a->orig == ip->class_Ptr )
        return a;
    TODO;
    return 0;
}

template<class TO>
static Type *type_promote( Type *a, TO ) {
    if ( TO::b )
        return ip->type_Bool;
    return a;
}

/**
*/
template<class TO>
struct Op : Inst {
    virtual void write_dot( Stream &os ) { os << op; }
    virtual operator BoolOpSeq() const {
        TODO;
        return BoolOpSeq();
    }
    TO op;
};

/**
*/
template<class TO>
struct UOp : Op<TO> {
    virtual Type *type() { return type_promote( this->inp[ 0 ]->type(), this->op ); }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new UOp<TO>; }
};


template<class TO>
static Expr _op( Expr a, TO ) {
    if ( a.error() )
        return ip->error_var();
    UOp<TO> *res = new UOp<TO>();
    res->add_inp( a );
    return res;
}

template<class TO>
static Expr _gp( Expr a, TO op ) {
    if ( a->type()->aryth )
        return _op( a, op );
    TODO;
    return 0;
}

/**
*/
template<class TO>
struct BOp : Op<TO> {
    virtual Type *type() { return type_promote( this->inp[ 0 ]->type(), this->inp[ 1 ]->type(), this->op ); }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new BOp<TO>; }
    virtual void set( Expr obj, const BoolOpSeq &cond ) {
        if ( this->flags & Inst::CONST )
            return ip->disp_error( "attempting to modify a const value" );
        if ( SameType<TO,Op_add>::res )
            return this->inp[ 0 ]->set( repl_bits( this->inp[ 0 ]->get( cond ), this->inp[ 1 ]->simplified( cond ), obj->simplified( cond ) ), cond );
        return Inst::set( obj, cond );
    }
    virtual Expr get( const BoolOpSeq &cond ) {
        if ( SameType<TO,Op_add>::res ) {
            Type *tr = this->inp[ 0 ]->ptype();
            return slice( tr, this->inp[ 0 ]->get( cond ), this->inp[ 1 ] );
        }
        return Inst::get( cond );
    }
    virtual void write( Codegen_C *cc, int prec ) {
        cc->on.write_beg();
        this->out_reg->write( cc, this->new_reg ) << " = ";
        if ( not TO::is_oper ) { to.write_oper( *cc->os ); *cc->os << "( "; }
        cc->write_out( this->inp[ 0 ] );
        if ( not TO::is_oper ) *cc->os << ", ";
        else { *cc->os << " "; to.write_oper( *cc->os ); *cc->os << " "; }
        cc->write_out( this->inp[ 1 ] );
        if ( not TO::is_oper ) *cc->os << " )";
        cc->on.write_end( ";" );
    }
    TO to;
};

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    template<class TA,class TB> \
    static Expr _op_cst_bin( TA *da, TB *db, Op_##NAME ) { return *da GEN *db; }
#include "DeclOp_Binary_Oper.h"
#undef DECL_OP

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    template<class TA,class TB> \
    static Expr _op_cst_bin( TA *da, TB *db, Op_##NAME ) { return GEN( *da, *db ); }
#include "DeclOp_Binary_Func.h"
#undef DECL_OP

template<class TA,class OP>
static Expr _op_cst_bin( TA *da, Type *tb, PI8 *db, OP op ) {
    #define DECL_BT( T ) if ( tb == ip->type_##T ) return _op_cst_bin( da, reinterpret_cast<T *>( db ), op );
    #include "DeclArytTypes.h"
    #undef DECL_BT
    return Expr();
}

// cst op ...
template<class TA,class TO>
Expr _op_cst_unk( Type *ta, TA *a, Expr b, TO ) {
    return (Inst *)0;
}
template<class TA,class TO>
Expr _op_cst_unk( Type *ta, TA *a, Expr b, Op_add o ) {
    if ( *a == 0 )
        return conv( type_promote( ta, b->type(), o ), b );
    return (Inst *)0;
}
template<class TA,class TO>
Expr _op_cst_unk( Type *ta, TA *a, Expr b, Op_mul o ) {
    if ( *a == 0 ) {
        Type *tr = type_promote( ta, b->type(), o );
        return cst( tr, tr->size(), 0 );
    }
    if ( *a == 1 )
        return conv( type_promote( ta, b->type(), o ), b );
    return (Inst *)0;
}

// ... op cst
template<class TB,class TO>
Expr _op_unk_cst( Expr a, Type *tb, TB *b, TO ) {
    return (Inst *)0;
}
template<class TB>
Expr _op_unk_cst( Expr a, Type *tb, TB *b, Op_add o ) {
    if ( *b == 0 )
        return conv( type_promote( a->type(), tb, o ), a );
    return (Inst *)0;
}
template<class TB>
Expr _op_unk_cst( Expr a, Type *tb, TB *b, Op_mul o ) {
    if ( *b == 0 ) {
        Type *tr = type_promote( a->type(), tb, o );
        return cst( tr, tr->size(), 0 );
    }
    if ( *b == 1 )
        return conv( type_promote( a->type(), tb, o ), a );
    return (Inst *)0;
}

//
template<class TO>
static Expr _op( Expr a, Expr b, TO op ) {
    if ( a.error() or b.error() )
        return ip->error_var();
    Type *ta = a->type();
    Type *tb = b->type();
    //ASSERT(  and tb->aryth, "..." );
    PI8 da[ ta->sb() ], db[ tb->sb() ];
    if ( ta->aryth and a->get_val( ta, da ) ) {
        if ( b->get_val( tb, db ) ) {
            #define DECL_BT( T ) if ( ta == ip->type_##T ) if ( Expr res = _op_cst_bin( reinterpret_cast<T *>( da ), tb, db, op ) ) return res;
            #include "DeclArytTypes.h"
            #undef DECL_BT
        }
        #define DECL_BT( T ) if ( ta == ip->type_##T ) if ( Expr res = _op_cst_unk( ta, reinterpret_cast<T *>( da ), b, op ) ) return res;
        #include "DeclArytTypes.h"
        #undef DECL_BT
    } else if ( b->get_val( tb, db ) ) {
        #define DECL_BT( T ) if ( tb == ip->type_##T ) if ( Expr res = _op_unk_cst( a, tb, reinterpret_cast<T *>( db ), op ) ) return res;
        #include "DeclArytTypes.h"
        #undef DECL_BT
    }

    BOp<TO> *res = new BOp<TO>();
    res->add_inp( a );
    res->add_inp( b );
    return res;
}

template<class TO>
static Expr _gp( Expr a, Expr b, TO op ) {
    if ( ( a->type()->aryth or a->type()->orig == ip->class_Ptr ) and b->type()->aryth )
        return _op( a, b, op );
    TODO;
    return 0;
}



#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    Expr op( Expr a, Expr b, Op_##NAME op ) { return _op( a, b, op ); } \
    Expr NAME( Expr a, Expr b ) { return _gp( a, b, Op_##NAME() ); }
#include "DeclOp_Binary.h"
#undef DECL_OP

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    Expr op( Expr a, Op_##NAME op ) { return _op( a, op ); } \
    Expr NAME( Expr a ) { return _gp( a, Op_##NAME() ); }
#include "DeclOp_Unary.h"
#undef DECL_OP

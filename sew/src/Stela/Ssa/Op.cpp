/****************************************************************************
**
** Copyright (C) 2014 SocaDB
**
** This file is part of the SocaDB toolkit/database.
**
** SocaDB is free software. You can redistribute this file and/or modify
** it under the terms of the Apache License, Version 2.0 (the "License").
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** You should have received a copy of the Apache License, Version 2.0
** along with this program. If not, see
** <http://www.apache.org/licenses/LICENSE-2.0.html>.
**
**
** Commercial License Usage
**
** Alternatively, licensees holding valid commercial SocaDB licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and SocaDB.
**
**
****************************************************************************/

#include "../Codegen/Codegen.h"
#include "../Codegen/OutReg.h"
#include "../System/SameType.h"
#include "../System/Math.h"
#include "ParsingContext.h"
#include "ReplBits.h"
#include "Slice.h"
#include "Type.h"
#include "Conv.h"
#include "Cst.h"
#include "Op.h"

template<class TO>
static Type *type_promote( Type *a, Type *b, TO ) {
    if ( TO::b )
        return ip->type_Bool;
    if ( a == b or a->orig == ip->class_Ptr )
        return a;
    if ( a == ip->type_Bool ) {
        return b;
    }
    if ( a == ip->type_SI32 ) {
        if ( b == ip->type_PI64 ) return ip->type_SI64;
        if ( b == ip->type_SI64 ) return b;
        if ( b == ip->type_FP32 ) return b;
        if ( b == ip->type_FP64 ) return b;
        return a;
    }
    if ( a == ip->type_SI64 ) {
        if ( b == ip->type_FP32 ) return b;
        if ( b == ip->type_FP64 ) return b;
        return a;
    }
    PRINT( *a );
    PRINT( *b );
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
    virtual void write_dot( Stream &os ) const { os << op; }
    virtual int op_num() const { return TO::op_id; }
    TO op;
};

/**
*/
template<class TO>
struct UOp : Op<TO> {
    virtual Type *type() { return type_promote( this->inp[ 0 ]->type(), this->op ); }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new UOp<TO>; }
    virtual int op_type() const {
        return to.op_id;
    }
    //    virtual void write( Codegen_C *cc ) {
    //        cc->on.write_beg();
    //        this->out_reg->write( cc, this->new_reg ) << " = ";
    //        to.write_oper( *cc->os );
    //        if ( not TO::is_oper ) *cc->os << "( ";
    //        else *cc->os << " ";
    //        cc->write_out( this->inp[ 0 ] );
    //        if ( not TO::is_oper ) *cc->os << " )";
    //        cc->on.write_end( ";" );
    //    }
    //    virtual operator Expr() {
    //        if ( SameType<TO,Op_not_boolean>::res )
    //            return not this->inp[ 0 ]->operator Expr();
    //        return Inst::operator Expr();
    //    }
    TO to;
};


template<class TO>
static Expr _simp_op( Expr a, TO to ) {
    return Expr();
}

template<class TO>
static Expr _op( Expr a, TO to ) {
    if ( a.error() )
        return a;

    // known value ?
    Type *ta = a->type();
    if ( ta->aryth ) {
        PI8 da[ ta->sb() ];
        if ( a->get_val( da, ta->sb() ) ) {
            #define DECL_BT( T ) if ( ta == ip->type_##T ) { return Expr( to( *reinterpret_cast<T *>( da ) ) ); }
            #include "DeclArytTypes.h"
            #undef DECL_BT
        }
    }

    // symbolic simplification
    if ( Expr res = _simp_op( a, to ) )
        return res;

    // else, make a new expr
    UOp<TO> *res = new UOp<TO>();
    res->add_inp( a );
    return Inst::twin_or_val( res );
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
    virtual void set( Expr obj, Expr cond ) {
        if ( this->flags & Inst::CONST )
            return ip->pc->disp_error( "attempting to modify a const value" );
        if ( SameType<TO,Op_add>::res )
            return this->inp[ 0 ]->set( repl_bits( this->inp[ 0 ]->get( cond ), this->inp[ 1 ]->simplified( cond ), obj->simplified( cond ) ), cond );
        return Inst::set( obj, cond );
    }
    virtual Expr get( Expr cond ) {
        if ( SameType<TO,Op_add>::res ) {
            Type *tr = this->inp[ 0 ]->ptype();
            return slice( tr, this->inp[ 0 ]->get( cond ), this->inp[ 1 ] );
        }
        return Inst::get( cond );
    }
    virtual Type *ptype() {
        if ( SameType<TO,Op_add>::res or SameType<TO,Op_sub>::res )
            return this->inp[ 0 ]->ptype(); // hum... not safe
        ip->pc->disp_error( "Not a pointer type" );
        ERROR( "Not a pointer type" );
        return 0;
    }
    virtual int op_type() const {
        return to.op_id;
    }

    virtual void write( Codegen *cc ) {
        if ( not this->out_reg ) {
            to.write_oper( cc->on.write_beg() );
            cc->on.write_end( " ??;" );
            return;
        }
        cc->on.write_beg() << *this->out_reg << " = ";
        cc->write_beg_cast_bop( this->out_reg->type );
        if ( SameType<TO,Op_mod>::res and ip->is_integer( this->out_reg->type ) ) {
            cc->write_out( this->inp[ 0 ] );
            *cc->os << " % ";
            cc->write_out( this->inp[ 1 ] );
        } else {
            if ( not TO::is_oper ) { to.write_oper( *cc->os ); *cc->os << "( "; }
            cc->write_out( this->inp[ 0 ] );
            if ( not TO::is_oper ) *cc->os << ", ";
            else { *cc->os << " "; to.write_oper( *cc->os ); *cc->os << " "; }
            cc->write_out( this->inp[ 1 ] );
            if ( not TO::is_oper ) *cc->os << " )";
        }
        cc->write_end_cast_bop( this->out_reg->type );
        cc->on.write_end( ";" );
        //        c->on << *c->var_decl( out_reg ) << " = " << *this << ";";
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
template<class TA>
Expr _op_cst_unk( Type *ta, TA *a, Expr b, Op_add o ) {
    if ( *a == 0 )
        return conv( type_promote( ta, b->type(), o ), b );
    return (Inst *)0;
}
template<class TA>
Expr _op_cst_unk( Type *ta, TA *a, Expr b, Op_and_boolean o ) {
    if ( *a )
        return b;
    return (Inst *)0;
}
template<class TA>
Expr _op_cst_unk( Type *ta, TA *a, Expr b, Op_or_boolean o ) {
    if ( not *a )
        return b;
    return (Inst *)0;
}
template<class TA>
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

template<class TO>
static Expr _simp_op( Expr a, Expr b, TO to ) {
    return Expr();
}

static Expr _simp_op( Expr a, Expr b, Op_equ ) {
    if ( a.inst == b.inst )
        return true;
    return Expr();
}

static Expr _simp_op( Expr a, Expr b, Op_neq ) {
    if ( a.inst == b.inst )
        return false;
    return Expr();
}

//
template<class TO>
static Expr _op( Expr a, Expr b, TO op ) {
    if ( a.error() or b.error() )
        return a;

    // constant values
    Type *ta = a->type();
    Type *tb = b->type();
    PI8 da[ ta->sb() ], db[ tb->sb() ];
    if ( ta->aryth and a->get_val( da, ta->size() ) ) {
        if ( b->get_val( db, tb ) ) {
            #define DECL_BT( T ) if ( ta == ip->type_##T ) if ( Expr res = _op_cst_bin( reinterpret_cast<T *>( da ), tb, db, op ) ) return res;
            #include "DeclArytTypes.h"
            #undef DECL_BT
        }
        #define DECL_BT( T ) if ( ta == ip->type_##T ) if ( Expr res = _op_cst_unk( ta, reinterpret_cast<T *>( da ), b, op ) ) return res;
        #include "DeclArytTypes.h"
        #undef DECL_BT
    } else if ( b->get_val( db, tb->size() ) ) {
        #define DECL_BT( T ) if ( tb == ip->type_##T ) if ( Expr res = _op_unk_cst( a, tb, reinterpret_cast<T *>( db ), op ) ) return res;
        #include "DeclArytTypes.h"
        #undef DECL_BT
    }

    // symbolic simplification
    if ( Expr res = _simp_op( a, b, op ) )
        return res;

    BOp<TO> *res = new BOp<TO>();
    res->add_inp( a );
    res->add_inp( b );
    return Inst::twin_or_val( res );
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

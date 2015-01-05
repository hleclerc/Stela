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

#include "ParsingContext.h"
#include "Slice.h"
#include "Rcast.h"
#include "Type.h"
#include "Op.h"

/**
   slice[ out_type ]( var, offset )
*/
struct Slice : Inst {
    Slice( Type *out_type ) : out_type( out_type ) {}
    virtual void write_dot( Stream &os ) const { os << "Slice"; }
    virtual void write_to_stream( Stream &os, int prec ) {
        int voff;
        if ( inp[ 1 ]->get_val( &voff, ip->type_SI32 ) and voff == 0 )
            os << "rcast[" << *out_type << "](" << inp[ 0 ] << ")";
        else
            Inst::write_to_stream( os, prec );
    }

    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Slice( out_type ); }
    virtual Type *type() { return out_type; }

    virtual Expr _simp_slice( Type *dst, Expr off ) {
        if ( Expr res = Inst::_simp_slice( dst, off ) )
            return res;
        return slice( dst, inp[ 0 ], add( off, inp[ 1 ] ) );
    }
    virtual Expr get( Expr cond ) {
        //        int voff;
        //        if ( inp[ 1 ]->get_val( ip->type_SI32, &voff ) and voff == 0 ) {
        //            if ( out_type->orig != ip->class_Ptr ) {
        //                PRINT( *out_type );
        //                ERROR( "" );
        //                return ip->ret_error( "expecting a ptr" );
        //            }
        //            Type *val_type = ip->type_from_type_var( out_type->parameters[ 0 ] );
        //            return rcast( val_type, inp[ 0 ]->get( cond ) );
        //        }
        TODO;
        return 0;
    }
    virtual void set( Expr obj, Expr cond ) {
        if ( inp[ 1 ]->always_equal( 0 ) )
            return inp[ 0 ]->set( obj, cond );
        TODO;
    }

    //    virtual void get_constraints() {
    //        if ( inp[ 0 ]->type()->size() == type()->size() ) {
    //            add_same_out( 0, this, -1, COMPULSORY ); // inp[ 0 ] <-> out
    //        }
    //    }

    //    virtual void codegen_simplification( Vec<Expr> &created, Vec<Expr> &out ) {
    //        if ( inp[ 0 ]->type()->size() == type()->size() )
    //            replace_this_by_inp( 0, out );
    //    }

    //    virtual void write( Codegen_C *cc ) {
    //        if ( out_reg == inp[ 0 ]->out_reg )
    //            return;
    //        Type *t = inp[ 1 ]->type();
    //        for( int i = 0; i < t->sb(); ++i ) {
    //            cc->on.write_beg() << "*( (char *)&";
    //            out_reg->write( cc, false ) << " + ";
    //            inp[ 1 ]->out_reg->write( cc, false ) << " / 8 + " << i << " ) = ";
    //            *cc->os << "*( (char *)&";
    //            inp[ 0 ]->out_reg->write( cc, false ) << " + " << i << " )";
    //            cc->on.write_end( ";" );
    //        }
    //    }

    Type *out_type;
};

Expr slice( Type *dst, Expr var, Expr off ) {
    if ( off->always_equal( 0 ) )
        return rcast( dst, var );

    if ( Expr res = var->_simp_slice( dst, off ) )
        return res;

    Slice *res = new Slice( dst );
    res->add_inp( var );
    res->add_inp( off );
    return Inst::twin_or_val( res );
}


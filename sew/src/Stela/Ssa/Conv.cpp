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

#include "Conv.h"

#include "Type.h"
#include "Cst.h"

/**
*/
struct Conv : Inst {
    Conv( Type *dst ) : dst( dst ) {}
    virtual void write_dot( Stream &os ) const { os << "Conv[" << *dst << "]"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Conv( dst ); }
    virtual Type *type() { return dst; }
    virtual Expr _subs() { return conv( dst, inp[ 0 ]->op_mp ); }
    //    virtual void write( Codegen_C *cc ) {
    //        if ( not this->out_reg ) {
    //            cc->on << "conv reg pb";
    //            return;
    //        }
    //        cc->on.write_beg();
    //        out_reg->write( cc, new_reg ) << " = ";
    //        bool p = out_reg->type != dst;
    //        if ( p ) {
    //            *cc->os << *dst << "(";
    //        }
    //        cc->write_out( inp[ 0 ] );
    //        if ( p )
    //            *cc->os << ")";
    //        cc->on.write_end( ";" );
    //    }
    Type *dst;
};

Expr conv( Type *dst, Expr inp ) {
    if ( dst == inp->type() )
        return inp;

    if ( dst->aryth ) {
        PI8 val[ dst->sb() ];
        if ( inp->get_val( val, dst ) )
            return cst( dst, dst->size(), val );
    }

    Conv *res = new Conv( dst );
    res->add_inp( inp );
    return Inst::twin_or_val( res );
}


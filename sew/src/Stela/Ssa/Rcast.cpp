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


#include "GlobalVariables.h"
#include "ParsingContext.h"
#include "Rcast.h"
#include "Type.h"
#include "Cst.h"

struct Rcast : Inst {
    Rcast( Type *type ) : _type( type ) {
    }
    virtual void write_dot( Stream &os ) const {
        os << "rcast";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Rcast( _type );
    }
    virtual Type *type() {
        return _type;
    }
    virtual Type *ptype() {
        if ( _type->orig == ip->class_Ptr )
            return ip->pc->type_from_type_expr( _type->parameters[ 0 ] );
        TODO;
        return 0;
    }
    virtual void set( Expr obj, Expr cond ) {
        if ( _type->orig == ip->class_Ptr ) {
            inp[ 0 ]->set( obj, cond );
            return;
        }
        TODO;
    }
    virtual Expr _simp_slice( Type *dst, Expr off ) {
        TODO;
        return Expr();
    }
    virtual Expr _simp_rcast( Type *dst ) {
        return rcast( dst, inp[ 0 ] );
    }
    virtual Expr get( Expr cond ) {
        TODO;
        return Expr();
    }

    Type *_type;
};

Expr rcast( Type *type, Expr val ) {
    if ( Expr res = val->_simp_rcast( type ) )
        return res;

    // else, create a new one
    Rcast *res = new Rcast( type );
    res->add_inp( val );
    return Inst::twin_or_val( res );
}

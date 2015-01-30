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


#include "../System/dcast.h"
#include "CstComputedSize.h"
#include "GlobalVariables.h"
#include "Type.h"
#include "Cst.h"

struct CstComputedSize : Inst {
    CstComputedSize( Type *type ) : _type( type ) {
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new CstComputedSize( _type );
    }
    virtual void write_dot( Stream &os ) const {
        os << "CstCompSize[" << *_type << "]";
    }
    virtual Type *type() {
        return _type;
    }
    virtual Bool _same_op( Inst *b ) { return _type == static_cast<CstComputedSize *>( b )->_type; }
    virtual void write( Codegen *c ) {
        //        c->on << *c->var_decl( out_reg ) << " = " << *this << ";";
        TODO;
    }
    virtual int op_type() const { return ID_OP_CstComputedSize; }
    virtual bool eq_twin_or_val( const Inst *inst ) {
        if ( const CstComputedSize *c = dcast( inst ) )
            return c->_type == _type;
        return false;
    }

    Type *_type;
};

Expr cst_computed_size( Type *type, Expr size ) {
    SI64 ksize;
    if ( size->get_val( &ksize, ip->type_SI64 ) )
        return cst( type, ksize, 0 );

    // else, create a new one
    CstComputedSize *res = new CstComputedSize( type );
    res->add_inp( size );
    return res;
}


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

#include "GetNout.h"

/**
*/
struct GetNout : Inst {
    GetNout( int nout ) : nout( nout ) {}
    virtual void write_dot( Stream &os ) const { os << "nout_" << nout; }
    virtual int op_type() const { return ID_OP_GetNout; }
    virtual Bool _same_op( Inst *b ) { return nout == static_cast<GetNout *>( b )->nout; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new GetNout( nout ); }
    virtual Type *ptype() { return inp[ 0 ]->ptype( nout ); }
    virtual Type *type() { return inp[ 0 ]->type( nout ); }
    virtual int pointing_to_nout() { return nout; }
    int nout;
};

Expr get_nout( Expr inp, int nout ) {
    GetNout *res = new GetNout( nout );
    res->add_inp( inp );
    return Inst::twin_or_val( res );
}


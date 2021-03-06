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

#include "UnknownInst.h"

/**
*/
struct UnknownInst : Inst {
    UnknownInst( Type *type, int num ) : out_type( type ), num( num ) {}
    virtual void write_dot( Stream &os ) const { os << "unk_" << num; }
    virtual int op_type() const { return ID_OP_UnknownInst; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new UnknownInst( out_type, num ); }
    virtual Type *type() { return out_type; }
    virtual Bool _same_op( Inst *b ) { return out_type == static_cast<UnknownInst *>( b )->out_type and num == static_cast<UnknownInst *>( b )->num; }
    Type *out_type;
    int num;
};

Expr unknown_inst( Type *type, int num ) {
    return new UnknownInst( type, num );
}


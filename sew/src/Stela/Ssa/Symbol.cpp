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


#include "Symbol.h"

/**
*/
struct Symbol : Inst {
    Symbol( Type *type, String name ) : out_type( type ), name( name ) {}
    virtual void write_dot( Stream &os ) const { os << name; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Symbol( out_type, name ); }
    virtual Type *type() { return out_type; }
    Type  *out_type;
    String name;
};

Expr symbol( Type *type, String name ) {
    Symbol *res = new Symbol( type, name );
    return res;
}


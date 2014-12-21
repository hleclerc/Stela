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


#ifndef CODEGEN_JS_H
#define CODEGEN_JS_H

#include "Codegen.h"

class Type;

/**
*/
class Codegen_Js : public Codegen {
public:
    Codegen_Js();

    void              gen_type( Stream &out, Type *type );

    virtual void      write_to( Stream &os );
    virtual Vec<Expr> make_code();
    virtual void      exec();

    virtual AutoPtr<Writable> var_decl( OutReg *reg );

    void              write_expr( Expr expr );
    Inst             *scheduling( Vec<Expr> &out );
};

#endif // CODEGEN_JS_H

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

class InstBlock;
class Type;

/**
*/
class Codegen_Js : public Codegen {
public:
    Codegen_Js();

    virtual void      write_to( Stream &os );
    virtual Vec<Expr> make_code();
    virtual void      exec();

    virtual void      write_beg_cast_bop( Type *type );
    virtual void      write_end_cast_bop( Type *type );
    virtual void      write_decl( Type *type, const Vec<OutReg *> &regs );

    void              write_expr( Expr expr );
    void              makeifinst( Vec<Expr> &out );
    void              scheduling( InstBlock &inst_block, Vec<Expr> &out );
    void              make_reg_decl( InstBlock &inst_block );

};

#endif // CODEGEN_JS_H

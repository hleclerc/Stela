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

#ifndef EXPR_H
#define EXPR_H

#include "../System/Stream.h"
class Inst;

/**
  Pointer to an Inst
*/
class Expr {
public:
    Expr( const Expr &obj );
    Expr( Inst *inst = 0 );
    Expr( SI64 val );
    ~Expr();

    Expr &operator=( const Expr &obj );

    bool operator==( const Expr &expr ) const;
    bool operator!=( const Expr &expr ) const { return not operator==( expr ); }
    bool operator<( const Expr &expr ) const { return inst < expr.inst; }
    operator bool() const { return inst; }

    const Inst *operator->() const { return inst; }
    Inst *operator->() { return inst; }

    const Inst &operator*() const { return *inst; }
    Inst &operator*() { return *inst; }

    void write_to_stream( Stream &os ) const;

    bool error();

    Inst *inst;
};

#endif // EXPR_H

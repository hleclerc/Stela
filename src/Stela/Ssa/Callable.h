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


#ifndef CALLABLE_H
#define CALLABLE_H

#include "Inst.h"

class ParsingContext;
class Ast_Callable;
class Varargs;

/**
*/
class Callable {
public:
    struct Trial {
        Trial( const char *reason = 0 );
        virtual ~Trial();

        virtual Expr call( int nu, Expr *vu, int nn, const String *names, Expr *vn, int pnu, Expr *pvu, int pnn, const String *pnames, Expr *pvn, int apply_mode, ParsingContext *ParsingContext, const Expr &cond, Expr self, Varargs *va_size_init );
        Trial *wr( const char *r ) { reason = r; return this; }
        bool ok() const { return not reason; }

        Expr        cond;
        const char *reason;
        double      computed_pertinence;
    };

    Callable( const Ast_Callable *ast_item );
    virtual Trial *test( int nu, Expr *vu, int nn, const String *names, Expr *vn, int pnu, Expr *pvu, int pnn, const String *pnames, Expr *pvn, ParsingContext *caller, Expr self ) = 0;

    const Ast_Callable *ast_item;
};

#endif // CALLABLE_H

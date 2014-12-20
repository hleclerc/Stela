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


#ifndef CLASS_H
#define CLASS_H

#include "../System/Vec.h"
#include "ParsingContext.h"
#include "Callable.h"
class Type;

/**
  Internal representation of a class variable. One day or another, it will be replaced by a pure Stela representation
*/
class Class : public Callable {
public:
    struct TrialClass : Trial {
        TrialClass( ParsingContext *caller, Class *orig );
        virtual ~TrialClass();
        virtual Expr call( int nu, Expr *vu, int nn, String *names, Expr *vn, int pnu, Expr *pvu, int pnn, String *pnames, Expr *pvn, int apply_mode, ParsingContext *caller, const Expr &cond, Expr self );
        ParsingContext ns;
        Class *orig;
    };

    Class( const Ast_Callable *ast_item = 0 );
    virtual Trial *test( int nu, Expr *vu, int nn, String *names, Expr *vn, int pnu, Expr *pvu, int pnn, String *pnames, Expr *pvn, ParsingContext *caller, Expr self );
    Type *type_for( Vec<Expr> args );

    Vec<Type *,-1,1> types;
};

#endif // CLASS_H

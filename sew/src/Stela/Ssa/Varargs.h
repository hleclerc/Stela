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


#ifndef VARARGS_H
#define VARARGS_H

#include "Inst.h"

/**
*/
class Varargs {
public:
    Vec<Expr  > exprs;
    Vec<String> names;

    int     nu() const { return exprs.size() - names.size(); } ///< nb unnamed
    int     nn() const { return names.size(); } ///< nb named
    Expr   *ua() { return exprs.ptr(); } ///< unnamed arguments
    Expr   *na() { return exprs.ptr() + nu(); } ///< amed arguments
    String *ns() { return names.ptr(); } ///< names (named strings)

    Vec<Expr> u_args() const;
    Vec<Expr> n_args() const;

    void append( Varargs &var );
};

#endif // VARARGS_H

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

// name, C++ op, is_oper, return bool, prec
DECL_OP( or_boolean , or , 1, 1, 14 )
DECL_OP( and_boolean, and, 1, 1, 15 )
DECL_OP( add        , +  , 1, 0, 20 )
DECL_OP( sub        , -  , 1, 0, 20 )
DECL_OP( mul        , *  , 1, 0, 21 )
DECL_OP( div        , /  , 1, 0, 21 )

DECL_OP( equ        , == , 1, 1, 17 )
DECL_OP( neq        , != , 1, 1, 17 )
DECL_OP( inf        , <  , 1, 1, 17 )
DECL_OP( sup        , >  , 1, 1, 17 )
DECL_OP( inf_eq     , <= , 1, 1, 17 )
DECL_OP( sup_eq     , >= , 1, 1, 17 )

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


#ifndef OP_H
#define OP_H

#include "../System/Math.h"
#include "OpTypes.h"
#include "Inst.h"


#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    struct Op_##NAME { void write_to_stream( Stream &os ) const { os << #NAME; } enum { is_oper = OPER, n = 2, prec = PREC, b = BOOL, op_id = ID_OP_##NAME }; void write_oper( Stream &os ) const { os << #GEN; } }; \
    Expr op( Expr a, Expr b, Op_##NAME ); \
    Expr NAME( Expr a, Expr b ); \
    enum { PREC_##NAME = PREC };
#include "DeclOp_Binary.h"
#undef DECL_OP

#define DECL_OP( NAME, GEN, OPER, BOOL, PREC ) \
    struct Op_##NAME { void write_to_stream( Stream &os ) const { os << #NAME; } enum { is_oper = OPER, n = 1, prec = PREC, b = BOOL, op_id = ID_OP_##NAME }; void write_oper( Stream &os ) const { os << #GEN; } template<class TA> auto operator()( TA a ) -> decltype( GEN( a ) ) const { return GEN( a ); } }; \
    Expr op( Expr a, Op_##NAME ); \
    Expr NAME( Expr a ); \
    enum { PREC_##NAME = PREC };
#include "DeclOp_Unary.h"
#undef DECL_OP


#endif // OP_H

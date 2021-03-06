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

#include "../System/SizeofIf.h"
#include "../System/Stream.h"
#include "ParsingContext.h"
#include "SurdefList.h"
#include "Inst.h"
#include "Cst.h"

Expr::Expr( const Expr &obj ) : inst( obj.inst ) {
    if ( inst ) ++inst->cpt_use;
}

Expr::Expr( Inst *inst ) : inst( inst ) {
    if ( inst ) ++inst->cpt_use;
}

#define DECL_BT( T ) Expr::Expr( T val ) : Expr( cst( ip->type_##T, SizeofIf<T,true>::val, &val ) ) {}
#include "DeclArytTypes.h"
#undef DECL_BT

Expr::~Expr() {
    if ( inst and --inst->cpt_use <= 0 )
        delete inst;
}

Expr &Expr::operator=( const Expr &obj ) {
    if ( obj.inst )
        ++obj.inst->cpt_use;
    if ( inst and --inst->cpt_use <= 0 )
        delete inst;
    inst = obj.inst;
    return *this;
}

bool Expr::equal( const Expr &expr ) const {
    if ( inst and expr.inst and inst->ptype() == ip->type_SurdefList and expr.inst->ptype() == ip->type_SurdefList ) {
        Expr ea =      inst->get( ip->pc->cond );
        Expr eb = expr.inst->get( ip->pc->cond );
        SI64 pa, pb;
        if ( ea->get_val( &pa, 64 ) and eb->get_val( &pb, 64 ) ) {
            SurdefList *sa = reinterpret_cast<SurdefList *>( ST( pa ) );
            SurdefList *sb = reinterpret_cast<SurdefList *>( ST( pb ) );
            return sa->equal( *sb );
        }
    }
    return inst == expr.inst;
}

bool Expr::operator==( const Expr &expr ) const {
    return inst == expr.inst;
}

void Expr::write_to_stream( Stream &os ) const {
    if ( inst )
        os << *inst;
    else
        os << "NULL";
}

bool Expr::error() {
    return inst == 0; // or inst->type() == ip->type_Error;
}

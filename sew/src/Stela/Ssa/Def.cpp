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


#include "Inst.h"
#include "Def.h"

Def::TrialDef::TrialDef( ParsingContext *caller, Def *orig ) : ns( caller ), orig( orig ), ns( 0, caller, "TrialDef_" + to_string( orig ) ) {
    // ns.catched_vars = &orig->catched_vars;
}

Def::TrialDef::~TrialDef() {
}

Expr Def::TrialDef::call( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, int apply_mode, ParsingContext *caller, const Expr &cond, Expr self ) {
    TODO;
    return Expr();
}

Def::Def( Ast_Callable *ast_item ) : Callable( ast_item ) {
}

Callable::Trial *Def::test( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, ParsingContext *caller, Expr self ) {
    TODO;
    if ( ast_item->pertinence )
        TODO;
    return 0;
}

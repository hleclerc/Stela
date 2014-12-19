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


#include "../Ast/Ast_Class.h"
#include "Class.h"

Class::TrialClass::TrialClass( ParsingContext *caller, Class *orig ) : ns( 0, caller, "TrialClass_" + to_string( orig ) ), orig( orig ) {
    // ns.catched_vars = &orig->catched_vars;
}

Class::TrialClass::~TrialClass() {
}

Expr Class::TrialClass::call( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, int apply_mode, ParsingContext *caller, const Expr &cond, Expr self ) {
    TODO;
    return Expr();
}

Class::Class( const Ast_Callable *ast_item ) : Callable( ast_item ) {
}

Callable::Trial *Class::test( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, ParsingContext *caller, Expr self ) {
    TODO;
    if ( ast_item->pertinence )
        TODO;
    return 0;
}

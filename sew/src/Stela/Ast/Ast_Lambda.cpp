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


#include "../Ssa/GlobalVariables.h"
#include "../Ssa/Room.h"
#include "../Ssa/Cst.h"
#include "../Ir/Numbers.h"
#include "Ast_Lambda.h"

Ast_Lambda::Ast_Lambda( int off ) : Ast( off ) {
}

void Ast_Lambda::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    TODO;
}

void Ast_Lambda::prep_get_potentially_needed_ext_vars( std::set<String> &avail ) const {
    TODO;
}

void Ast_Lambda::write_to_stream( Stream &os, int nsp ) const {
    os << "Lambda";
}

Expr Ast_Lambda::_parse_in( ParsingContext &context ) const {
    SI64 res[ 2 ];
    res[ 0 ] = ST( this );
    res[ 1 ] = 0;
    return room( cst( ip->type_Lambda, 2 * 64, res ) );
}

void Ast_Lambda::_get_info( IrWriter *aw ) const {
    TODO;
}

PI8 Ast_Lambda::_tok_number() const {
    return IR_TOK_LAMBDA;
}

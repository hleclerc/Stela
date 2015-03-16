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

#include "IpSnapshot.h"

int IpSnapshot::cur_date = 0;

IpSnapshot::IpSnapshot( IpSnapshot *&prev ) : iptr( prev ), prev( prev ), date( ++cur_date ) {
    iptr = this;
}

void IpSnapshot::reg_parsing_context( ParsingContext *s ) {
    StatePS sp;
    sp.ret        = s->ret;
    sp.cond       = s->cond;
    sp.cont       = s->cont;
    sp.rem_breaks = s->rem_breaks;
    parsing_contexts.insert( std::make_pair( s, sp ) );
}

void IpSnapshot::undo_parsing_contexts() {
    for( std::pair<ParsingContext *,StatePS> o : parsing_contexts ) {
        o.first->ret        = o.second.ret;
        o.first->cond       = o.second.cond;
        o.first->cont       = o.second.cont;
        o.first->rem_breaks = o.second.rem_breaks;
    }
}


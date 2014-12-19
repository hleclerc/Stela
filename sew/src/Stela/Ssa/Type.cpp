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

#include "../Ast/Ast_Callable.h"
#include "../System/Assert.h"
#include "Class.h"
#include "Type.h"

Type::Type( Class *orig ) : orig( orig ) {
    _len = -1;
    _ali = -1;
    _pod = -1;

    aryth = false;
}

void Type::write_to_stream( Stream &os ) const {
    if ( this and orig and orig->ast_item )
        os << orig->ast_item->name;
    else
        os << "NULL";
}

int Type::size() {
    if ( _len < 0 )
        parse();
    return _len;
}

int Type::sb() {
    return size() >= 0 ? ( size() + 7 ) / 8 : -1;
}

bool Type::always( bool val, const PI8 *data, const PI8 *knwn ) {
    TODO;
    return false;
}

bool Type::always_equal( Type *t, const void *d, const PI8 *data, const PI8 *knwn ) {
    TODO;
    return false;
}

void Type::write_val( Stream &os, const PI8 *data, const PI8 *knwn ) {
    // if ( orig and )
    int len = size();
    if ( len < 0 )
        TODO;
    const char *c = "0123456789ABCDEF";
    for( int i = 0; i < std::min( len / 8, 4 ); ++i ) {
        if ( i )
            os << ' ';
        os << c[ data[ i ] >> 4 ] << c[ data[ i ] & 0xF ];
    }
    if ( len / 8 > 4 )
        os << "...";
}

void Type::parse() {
    TODO;
}

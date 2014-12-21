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
#include "../System/Math.h"
#include "Class.h"
#include "Type.h"
#include "Op.h"

Type::Type( Class *orig ) : orig( orig ) {
    _len    = -1;
    _ali    = -1;
    _pod    = -1;
    _parsed = false;
    aryth   = false;
}

void Type::write_to_stream( Stream &os ) const {
    if ( this and orig and orig->ast_item )
        os << orig->ast_item->name;
    else
        os << "NULL";
}

int Type::alig() {
    if ( _ali < 0 )
        parse();
    return _ali;
}

int Type::size() {
    if ( _len < 0 )
        parse();
    return _len;
}

int Type::sb() {
    return size() >= 0 ? ( size() + 7 ) / 8 : -1;
}

Expr Type::size( Expr obj ) {
    int s = size();
    if ( s >= 0 )
        return Expr( s );
    Expr res( 0 );
    for( Attr &attr : attributes ) {
        TODO;
    }
    return res;
}

bool Type::get_val( void *res, Type *type, const PI8 *data, const PI8 *knwn ) {
    return false;
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
    if ( _parsed )
        return;
    _parsed = true;

    // parse block
    ParsingContext ns( 0, 0, "parsing type " + to_string( this ) );
    // ns.class_scope = this;
    ASSERT( orig->ast_item->arguments.size() == parameters.size(), "..." );
    for( int i = 0; i < parameters.size(); ++i )
        ns.reg_var( orig->ast_item->arguments[ i ], parameters[ i ], true );
    orig->ast_item->block->parse_in( ns );

    _len = ns.base_size;
    _ali = ns.base_alig;

    for( ParsingContext::NamedVar &nv : ns.variables ) {
        if ( nv.expr->flags & Inst::SURDEF ) {
            attributes << Attr{ -1, nv.expr, nv.name };
        } else {
            _ali = ppcm( _ali, nv.expr->ptype()->alig() );
            _len = ceil( _len, _ali );

            attributes << Attr{ _len, nv.expr, nv.name };

            if ( nv.expr->ptype()->size() < 0 )
                TODO;
            _len += nv.expr->ptype()->size();
        }
    }

    for( ParsingContext::NamedVar &nv : *ns.static_variables )
        attributes << Attr{ -1, nv.expr, nv.name };
}

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


#include "Varargs.h"

Varargs::Varargs( int rese_nu, int rese_nn ) : exprs( Rese(), rese_nu + rese_nn ), names( Rese(), rese_nn ) {
}

void Varargs::write_to_stream( Stream &os ) {
    for( int i = 0; i < nu(); ++i )
        os << ua()[ i ] << " ";
    for( int i = 0; i < nn(); ++i )
        os << names[ i ] << "=" << na()[ i ] << " ";
}

bool Varargs::equal( const Varargs &s ) const {
    return exprs.equal( s.exprs ) and names == s.names;
}

Expr Varargs::find( String n ) {
    for( int i = 0; i < nn(); ++i )
        if ( names[ i ] == n )
            return na()[ i ];
    return Expr();
}

void Varargs::push_named( String n, Expr e ) {
    names << n;
    exprs << e;
}

Vec<Expr> Varargs::u_args() const {
    Vec<Expr> res( Rese(), nu() );
    for( int i = 0; i < nu(); ++i )
        res << exprs[ i ];
    return res;
}

Vec<Expr> Varargs::n_args() const {
    Vec<Expr> res( Rese(), nn() );
    for( int i = nu(); i < exprs.size(); ++i )
        res << exprs[ i ];
    return res;
}

void Varargs::append( Varargs &var ) {
    Vec<Expr  > n_exprs( Rese(), exprs.size() + var.exprs.size() );
    Vec<String> n_names( Rese(), names.size() + var.names.size() );

    // unnamed
    for( int i = 0; i < nu(); ++i )
        n_exprs << ua()[ i ];
    for( int i = 0; i < var.nu(); ++i )
        n_exprs << var.ua()[ i ];
    // named
    for( int i = 0; i < nn(); ++i ) {
        n_exprs << na()[ i ];
        n_names << ns()[ i ];
    }
    for( int i = 0; i < var.nn(); ++i ) {
        n_exprs << var.na()[ i ];
        n_names << var.ns()[ i ];
    }

    exprs = n_exprs;
    names = n_names;
}

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


#include "Cond.h"
#include "Op.h"

static void get_and_comp( Vec<Expr> &res, Expr a ) {
    if ( a->op_type() == ID_OP_and_boolean ) {
        get_and_comp( res, a->inp[ 0 ] );
        get_and_comp( res, a->inp[ 1 ] );
    } else
        res << a;
}

static Expr make_and_boolean( Vec<Expr> &res, int o = 0 ) {
    if ( o == res.size() )
        return true;
    if ( o + 1 == res.size() )
        return res[ o ];
    return and_boolean( res[ o ], make_and_boolean( res, o + 1 ) );
}

Expr cond_sub( Expr a, Expr b ) {
    Vec<Expr> vec_a, vec_b;
    get_and_comp( vec_a, a );
    get_and_comp( vec_b, b );
    for( int i = 0; i < vec_a.size(); ++i )
        if ( vec_b.contains( vec_a[ i ] ) )
            vec_a.remove( i-- );
    return make_and_boolean( vec_a );
}

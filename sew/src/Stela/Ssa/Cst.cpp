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

#include "../System/Memcpy.h"
#include "Type.h"
#include "Cst.h"
#include <map>

///
static std::map<Type *,Vec<class Cst *> > cst_set;

///
class Cst : public Inst {
public:
    Cst( Type *type, SI64 size, const void *value, const void *known ) : type( type ), size( size ) {
        int sb = ( size + 7 ) / 8;
        data.resize( 2 * sb );
        memcpy_bit( data.ptr() + 0 * sb, 0, value, 0, size );
        if ( known )
            memcpy_bit( data.ptr() + 1 * sb, 0, known, 0, size );
        else
            for( int i = 1 * sb; i < 2 * sb; ++i )
                data[ i ] = 0xFF;
        cst_set[ type ] << this;
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        int sb = ( size + 7 ) / 8;
        return new Cst( type, size, data.ptr(), data.ptr() + sb );
    }
    virtual ~Cst() {
        cst_set[ type ].remove_first_unordered( this );
    }
    virtual void write_to_stream( Stream &os ) const {
        write_dot( os );
    }
    virtual void write_dot( Stream &os ) const {
        type->write_val( os, data.ptr() );
    }

    Vec<PI8> data; ///< values and known (should not be changed directly)
    Type    *type;
    int      size;
};

static bool equal_cst( const Cst *cst, int size, const PI8 *value, const PI8 *known ) {
    if ( cst->size != size )
        return false;
    int sb = ( size + 7 ) / 8;
    for( int i = 0; i < sb; ++i )
        if ( ( cst->data[ i ] & cst->data[ sb + i ] ) != ( value[ i ] & cst->data[ sb + i ] ) )
            return false;
    if ( known ) {
        for( int i = 0; i < sb; ++i )
            if ( cst->data[ sb + i ] != known[ i ] )
                return false;
    } else {
        for( int i = 0; i < sb; ++i )
            if ( cst->data[ sb + i ] != 0xFF )
                return false;
    }
    return true;
}

Expr cst( Type *type, SI64 size, const void *value, const void *known ) {
    if ( size and not value ) {
        ASSERT( not known, "weird" );
        int sb = ( size + 7 ) / 8;
        PI8 nptr[ sb ];
        PI8 nkno[ sb ];
        for( int i = 0; i < sb; ++i ) {
            nptr[ i ] = 0x00;
            nkno[ i ] = 0x00;
        }
        return cst( type, size, nptr, nkno );
    }

    // already an equivalent cst ?
    for( Cst *c : cst_set[ type ] )
        if ( equal_cst( c, size, (const PI8 *)value, (const PI8 *)known ) )
            return c;

    // else, create a new one
    return new Cst( type, size, value, known );
}

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

#include "../Codegen/Codegen.h"
#include "../System/Memcpy.h"
#include "GlobalVariables.h"
#include "ParsingContext.h"
#include "Slice.h"
#include "Type.h"
#include "Cst.h"
#include <map>

///
static std::map<Type *,Vec<class Cst *> > cst_set;

///
class Cst : public Inst {
public:
    Cst( Type *type, SI64 size, const void *value, const void *known ) : _type( type ), _size( size ) {
        int sb = ( size + 7 ) / 8;
        _data.resize( 2 * sb );
        memcpy_bit( _data.ptr() + 0 * sb, 0, value, 0, size );
        if ( known )
            memcpy_bit( _data.ptr() + 1 * sb, 0, known, 0, size );
        else
            for( int i = 1 * sb; i < 2 * sb; ++i )
                _data[ i ] = 0xFF;
        cst_set[ type ] << this;
    }
    virtual ~Cst() {
        cst_set[ _type ].remove_first_unordered( this );
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        int sb = ( _size + 7 ) / 8;
        return new Cst( _type, _size, _data.ptr(), _data.ptr() + sb );
    }
    virtual void write_to_stream( Stream &os ) const {
        write_dot( os );
    }
    virtual void write_dot( Stream &os ) const {
        if ( _type == ip->type_Type )
            os << **reinterpret_cast<const Type * const *>( _data.ptr() );
        else
            _type->write_val( os, _data.ptr() );
    }
    virtual Type *type() {
        return _type;
    }
    virtual bool get_val( void *res, Type *type ) {
        int sb = ( _size + 7 ) / 8;
        return _type->get_val( res, type, _data.ptr(), _data.ptr() + sb );
    }
    virtual bool get_val( void *res, int size ) {
        if ( _size < size )
            return false;
        memcpy_bit( res, 0, _data.ptr(), 0, size );
        return true;
    }
    virtual bool always( bool val ) const {
        int sb = ( _size + 7 ) / 8;
        return _type->always( val, _data.ptr(), _data.ptr() + sb );
    }
    virtual bool always_equal( Type *t, const void *d ) {
        int sb = ( _size + 7 ) / 8;
        return _type->always_equal( t, d, _data.ptr(), _data.ptr() + sb );
    }
    virtual void write( Codegen *c ) {
        c->on << *c->var_decl( out_reg ) << " = " << *this << ";";
    }
    virtual Expr _simp_repl_bits( Expr off, Expr val ) {
        SI32 voff, vlen = val->type()->size();
        if ( vlen >= 0 and off->get_val( &voff, ip->type_SI32 ) ) {
            PI8 vval[ ( vlen + 7 ) / 8 ];
            if ( val->get_val( vval, vlen ) ) {
                ASSERT( voff + vlen <= _size, "..." );
                PI8 ndat[ ( _size + 7 ) / 8 ];
                memcpy_bit( ndat, 0, _data.ptr(), 0, _size ); // copy the old bits
                memcpy_bit( ndat, voff, vval, 0, vlen ); // copy new values
                return cst( _type, _size, ndat );
            }
        }
        return Inst::_simp_repl_bits( off, val );
    }
    virtual Expr _simp_slice( Type *dst, Expr off ) {
        SI32 voff;
        if ( off->get_val( &voff, ip->type_SI32 ) ) {
            SI32 sb = ( _size + 7 ) / 8, ns = _size - voff, nb = ( ns + 7 ) / 8;
            PI8 nd[ nb ];
            PI8 nk[ nb ];
            memcpy_bit( nd, 0, _data.ptr() + 0 * sb, voff, ns );
            memcpy_bit( nk, 0, _data.ptr() + 1 * sb, voff, ns );
            return cst( dst, ns, nd, nk );
        }
        return Expr();
    }

    virtual Expr _simp_rcast( Type *dst ) {
        int sb = ( _size + 7 ) / 8;
        return cst( dst, _size, _data.ptr(), _data.ptr() + sb );
    }

    Vec<PI8> _data; ///< values and known (should not be changed directly)
    Type    *_type;
    int      _size;
};

static bool equal_cst( const Cst *cst, Type *type, int size, const PI8 *value, const PI8 *known ) {
    if ( cst->_type != type )
        return false;
    if ( cst->_size != size )
        return false;
    int sb = ( size + 7 ) / 8;
    for( int i = 0; i < sb; ++i )
        if ( ( cst->_data[ i ] & cst->_data[ sb + i ] ) != ( value[ i ] & cst->_data[ sb + i ] ) )
            return false;
    if ( known ) {
        for( int i = 0; i < sb; ++i )
            if ( cst->_data[ sb + i ] != known[ i ] )
                return false;
    } else {
        for( int i = 0; i < sb; ++i )
            if ( cst->_data[ sb + i ] != 0xFF )
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
        if ( equal_cst( c, type, size, (const PI8 *)value, (const PI8 *)known ) )
            return c;

    // else, create a new one
    return new Cst( type, size, value, known );
}

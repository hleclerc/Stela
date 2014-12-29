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


#ifndef TYPE_H
#define TYPE_H

#include "../System/Stream.h"
#include "Inst.h"
class Class;

/**
*/
class Type {
public:
    struct Attr {
        int    off; ///< -1 => static; -2 => not known because of a repeated
        Expr   val; ///< flags may contain to Inst::PART_INST
        String name;
    };

    Type( Class *orig );

    void  write_to_stream( Stream &os ) const;
    int   alig(); ///< needed alignement in bits
    int   size(); ///< size in bits, or -1 if not known
    int   sb(); ///< size in bytes, or -1 if not known
    Expr  size( Expr obj ); ///< size in bits (return an expr to be computed if not known)
    void  find_attr( Vec<Attr *> &attr_list, String name );
    Attr *find_attr( String name );
    Expr  find_static_attr( String name );

    // numeric/known values
    virtual bool get_val( void *res, Type *type, const PI8 *data, const PI8 *knwn );
    virtual bool always( bool val, const PI8 *data, const PI8 *knwn );
    virtual bool always_equal( Type *t, const void *d, const PI8 *data, const PI8 *knwn );

    //


    virtual void write_val( Stream &os, const PI8 *data, const PI8 *knwn = 0 );
    void parse();

    Class    *orig;
    Vec<Expr> parameters;

    int       _len;
    int       _ali;
    int       _pod;
    bool      _parsed;
    bool      aryth;
    Vec<Attr> attributes;
};

#endif // TYPE_H

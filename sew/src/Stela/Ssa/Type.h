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
class Class;

/**
*/
class Type {
public:
    Type( Class *orig );
    void write_to_stream( Stream &os ) const;
    int  size(); ///< size in bits, or -1 if not known
    int  sb(); ///< size in bytes, or -1 if not known

    virtual bool always( bool val, const PI8 *data, const PI8 *knwn );
    virtual bool always_equal( Type *t, const void *d, const PI8 *data, const PI8 *knwn );

    virtual void write_val( Stream &os, const PI8 *data, const PI8 *knwn = 0 );
    void parse();

    Class *orig;
    int    _len;
    int    _ali;
    int    _pod;
    bool   aryth;
};

#endif // TYPE_H

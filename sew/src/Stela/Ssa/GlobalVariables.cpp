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

#include "GlobalVariables.h"
#include "Type.h"

template<class T>
struct Type_ : Type {
    virtual void write_val( Stream &os, const PI8 *data, const PI8 *knwn ) {
        os << *reinterpret_cast<const T *>( data );
    }
};

static Type_<SI32> _type_SI32;
static Type_<SI64> _type_SI64;

GlobalVariables::GlobalVariables() {
    type_SI32 = &_type_SI32;
    type_SI64 = &_type_SI64;
}

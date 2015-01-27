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


#ifndef TYPEGEN_JS_H
#define TYPEGEN_JS_H

#include "../System/StreamSep.h"
#include "../System/Stream.h"
#include "TypeGen.h"

/**
*/
class TypeGen_JS : public TypeGen {
public:
    TypeGen_JS( Type *type, Stream *ms = &std::cout );

    virtual void exec();

    void gen_func( FuncToGen &f );

    StreamSepMaker<Stream> on;
    Stream                *os; ///< currrent stream
};

#endif // TYPEGEN_JS_H

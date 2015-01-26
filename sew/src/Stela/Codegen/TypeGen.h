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


#ifndef TYPEGEN_H
#define TYPEGEN_H

#include "../Ssa/Inst.h"

/**
*/
class TypeGen {
public:
    struct FuncToGen {
        bool operator==( const FuncToGen &g ) const { return name == g.name and arg_types == g.arg_types; }
        String name;
        Vec<Type *> arg_types;
    };

    TypeGen( Type *type );
    void add_func_to_gen( String name, Vec<Type *> arg_types );

    virtual void exec() = 0;

protected:
    Type *type;
    Vec<FuncToGen> funcs_to_gen;
};

#endif // TYPEGEN_H

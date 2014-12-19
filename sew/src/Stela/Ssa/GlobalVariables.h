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

#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include "../System/ErrorList.h"
#include "Inst.h"
#include <map>
#include <set>
class ParsingContext;
class Class;

struct GlobalVariables {
    struct Variable {
        Expr expr;
    };

    GlobalVariables();

    ParsingContext                      *pc; ///< current parsing context
    ParsingContext                      *main_parsing_context;
    ErrorList                            error_list;
    Vec<String>                          include_paths;
    std::set<String>                     already_parsed;
    std::map<String,Vec<Variable,-1,1> > main_scope;

    #define DECL_BT( T ) Class *class_##T;
    #include "DeclParmClass.h"
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) Type *type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    Type *type_ST;
    int   ptr_size;

};

extern GlobalVariables *ip; ///<


#endif // GLOBALVARIABLES_H

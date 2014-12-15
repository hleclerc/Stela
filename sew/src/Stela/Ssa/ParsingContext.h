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


#ifndef PARSINGCONTEXT_H
#define PARSINGCONTEXT_H

#include "../System/ErrorList.h"
#include <set>

/**
*/
class ParsingContext {
public:
    struct GlobalVariables {
        ErrorList        error_list;
        Vec<String>      include_paths;
        std::set<String> already_parsed;
    };

    ParsingContext( GlobalVariables &gv );
    ParsingContext( ParsingContext *parent );

    void parse( String filename, String current_dir );

    void   add_inc_path( String path );
    String find_src( String filename, String current_dir = "" ) const;

    void              disp_error( String msg, bool warn = false, const char *file = 0, int line = -1 );
    ErrorList::Error &error_msg ( String msg, bool warn = false, const char *file = 0, int line = -1 );
    // Past              ret_error ( String msg, bool warn = false, const char *file = 0, int line = -1 );

    GlobalVariables &gv;

    ParsingContext  *parent;
    int              current_off;
};

#endif // PARSINGCONTEXT_H

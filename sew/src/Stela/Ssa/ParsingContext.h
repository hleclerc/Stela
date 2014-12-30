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

#include "GlobalVariables.h"
#include "Inst.h"

/**
*/
class ParsingContext {
public:
    typedef GlobalVariables::NamedVar NamedVar;
    enum {
        SCOPE_TYPE_CLASS, ///< first level (methods and attributes) of a class content
        SCOPE_TYPE_MAIN,
        SCOPE_TYPE_STD
    };

    ParsingContext( ParsingContext *parent = 0, ParsingContext *caller = 0, String add_scope_name = "" );

    void              parse( String filename, String current_dir );

    void              add_inc_path( String path );
    String            find_src( String filename, String current_dir = "" ) const;

    Expr              reg_var( String name, Expr expr, bool stat = false );
    Expr              get_var( String name, bool disp_err = true );

    enum              ApplyMode { APPLY_MODE_STD, APPLY_MODE_PARTIAL_INST, APPLY_MODE_NEW };
    Expr              apply( Expr f, int nu = 0, Expr *u_args = 0, int nn = 0, const String *n_name = 0, Expr *n_args = 0, ApplyMode am = APPLY_MODE_STD );
    Expr              make_type_var( Type *type );
    Expr              copy( Expr var );

    void              disp_error( String msg, bool warn = false, const char *file = 0, int line = -1 );
    ErrorList::Error &error_msg ( String msg, bool warn = false, const char *file = 0, int line = -1 );
    Expr              ret_error ( String msg, bool warn = false, const char *file = 0, int line = -1 );
    Expr              error_var ();


    Expr              _make_surdef_list( const Vec<Expr> &lst );
    Expr              _find_first_var_with_name( String name );
    void              _find_list_of_vars_with_name( Vec<Expr> &res, String name );

    ParsingContext  *parent;
    ParsingContext  *caller;
    Expr             self;
    Expr            *cont; ///< wether to continue or not
    String           scope_name; ///< used to find static scope
    int              current_off;
    Vec<NamedVar>    variables;
    Vec<NamedVar>   *static_variables;
    int              scope_type;
    Expr             cond;
    int              base_size;
    int              base_alig;
};


#endif // PARSINGCONTEXT_H

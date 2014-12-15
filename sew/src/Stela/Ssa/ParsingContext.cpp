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

#include "../System/FileExists.h"
#include "../System/ReadFile.h"
#include "../Met/Lexer.h"
#include "../Ast/Ast.h"
#include "ParsingContext.h"

ParsingContext::ParsingContext( GlobalVariables &gv ) : parent( 0 ), gv( gv ) {

}

ParsingContext::ParsingContext( ParsingContext *parent ) : parent( parent ), gv( gv ) {

}

void ParsingContext::add_inc_path( String path ) {
    gv.include_paths.push_back_unique( path );
}

void ParsingContext::parse( String filename, String current_dir ) {
    int old_error_list_size = gv.error_list.size();

    // find the file
    filename = find_src( filename, current_dir );
    if ( filename.size() == 0 or gv.already_parsed.count( filename ) )
        return;
    gv.already_parsed.insert( filename );

    // -> source data
    ReadFile r( filename.c_str() );
    if ( not r )
        return disp_error( "Impossible to open " + filename );

    // -> lexical data
    Lexer lexer( gv.error_list );
    lexer.parse( r.data, filename.c_str() );
    if ( gv.error_list.size() != old_error_list_size )
        return;

    // -> parse Ast
    Past ast = make_ast( gv.error_list, lexer.root(), true );
    // ast->parse_in( main_scope );
    PRINT( *ast );
}

String ParsingContext::find_src( String filename, String current_dir ) const {
    // absolute path ?
    if ( filename[ 0 ] == '/' or filename[ 0 ] == '\\' )
        return file_exists( filename ) ? filename : String();

    // try with current_dir
    if ( current_dir.size() ) {
        String trial = current_dir + '/' + filename;
        if ( file_exists( trial ) )
            return trial;
    }

    // try with inc_paths
    for( String path : gv.include_paths ) {
        String trial = path + '/' + filename;
        if ( file_exists( trial ) )
            return trial;
    }

    // not found :(
    return String();
}

void ParsingContext::disp_error( String msg, bool warn, const char *file, int line ) {
    std::cerr << error_msg( msg, warn, file, line );
}

ErrorList::Error &ParsingContext::error_msg( String msg, bool warn, const char *file, int line ) {
    ErrorList::Error &res = gv.error_list.add( msg, warn );
    if ( file )
        res.caller_stack.push_back( line, file );
    //for( int i = parse_stack.size() - 1; i >= 0; --i )
    //    res.ac( parse_stack[ i ].filename.c_str(), parse_stack[ i ].offset );
    return res;
}

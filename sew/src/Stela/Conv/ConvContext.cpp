#include "../System/FileExists.h"
#include "../System/StreamSep.h"
#include "../System/ReadFile.h"
#include "../Ast/Ast_Error.h"
#include "../Ast/Ast_Void.h"
#include "../Met/Lexer.h"
#include "ConvContext.h"
#include "ConvType.h"
#include <sstream>

ConvContext *cc;

ConvContext::ConvContext() {
}

void ConvContext::add_inc_path( String path ) {
    inc_paths.push_back_unique( path );
}

void ConvContext::parse( String filename, String current_dir ) {
    int old_error_list_size = error_list.size();

    // find the file
    filename = find_src( filename, current_dir );
    if ( filename.size() == 0 or sourcefiles.count( filename ) )
        return;

    // -> source data
    ReadFile r( filename.c_str() );
    if ( not r )
        return disp_error( "Impossible to open " + filename );

    // -> lexical data
    Lexer lexer( error_list );
    lexer.parse( r.data, filename.c_str() );
    if ( error_list.size() != old_error_list_size )
        return;

    // -> parse Ast
    file_stack << filename;
    Past ast = make_ast( error_list, lexer.root(), true );
    ast->parse_in( main_scope );
    file_stack.pop_back();

    sourcefiles[ filename ] = ast;
}

String ConvContext::find_src( String filename, String current_dir, bool allow_cur_dir ) const {
    // absolute path ?
    if ( filename[ 0 ] == '/' or filename[ 0 ] == '\\' )
        return file_exists( filename ) ? filename : String();

    // try with current_dir
    if ( allow_cur_dir and current_dir.size() ) {
        String trial = current_dir + '/' + filename;
        if ( file_exists( trial ) )
            return trial;
    }

    // try with inc_paths
    for( String path : inc_paths ) {
        String trial = path + '/' + filename;
        if ( file_exists( trial ) )
            return trial;
    }

    // not found :(
    return String();
}

//Expr ConvContext::ret_error( String msg, bool warn, const char *file, int line ) {
//    disp_error( msg, warn, file, line );
//    return error_var();
//}

void ConvContext::disp_error( String msg, bool warn, const char *file, int line ) {
    std::cerr << error_msg( msg, warn, file, line );
}

ErrorList::Error &ConvContext::error_msg( String msg, bool warn, const char *file, int line ) {
    ErrorList::Error &res = error_list.add( msg, warn );
    if ( file )
        res.caller_stack.push_back( line, file );
    for( int i = parse_stack.size() - 1; i >= 0; --i )
        res.ac( parse_stack[ i ].filename.c_str(), parse_stack[ i ].offset );
    return res;
}

Past ConvContext::ret_error( String msg, bool warn, const char *file, int line ) {
    disp_error( msg, warn, file, line );
    return error_var();
}

Past ConvContext::error_var() {
    return new Ast_Void;
}

Past ConvContext::void_var() {
    return new Ast_Error;
}

void ConvContext::out_code_for_class( Stream &os, String class_name ) {
    // TODO for class_name like Vec[PI8,10], update var with the arguments
    Past var = main_scope.find_var( class_name );
    if ( not var )
        return disp_error( "Impossible to find class " + class_name );

    ConvType *type = var->make_type();

    std::ostringstream ss;
    StreamSepMaker<Stream> on( &ss );
    type->write_cpp_decl( on );

    os << ss.str();
}


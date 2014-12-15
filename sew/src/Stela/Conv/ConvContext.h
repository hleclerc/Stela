#ifndef CONVCONTEXT_H
#define CONVCONTEXT_H

#include "../System/ErrorList.h"
#include "../Ast/Ast.h"
#include "ConvScope.h"
#include <map>

/**
   Conversion context
*/
class ConvContext {
public:
    struct ParseData {
        String filename;
        int    offset;
    };

    ConvContext();

    // src files
    void   add_inc_path( String path );
    void   parse( String filename, String current_dir );
    String find_src( String filename, String current_dir, bool allow_cur_dir = true ) const;

    // errors
    void              disp_error( String msg, bool warn = false, const char *file = 0, int line = -1 );
    ErrorList::Error &error_msg ( String msg, bool warn = false, const char *file = 0, int line = -1 );
    Past              ret_error ( String msg, bool warn = false, const char *file = 0, int line = -1 );

    // variables
    Past              error_var();
    Past              void_var();

    //
    void out_code_for_class( Stream &os, String class_name );

    std::map<String,Past> sourcefiles;
    ErrorList             error_list;
    Vec<String>           inc_paths;
    ConvScope             main_scope;
    Vec<String>           file_stack;
    Vec<ParseData>        parse_stack;
};

extern ConvContext *cc;

#endif // CONVCONTEXT_H

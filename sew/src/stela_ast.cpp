/**
  main file for the stela interpreter / compiler
*/
#include <Stela/Ssa/ParsingContext.h>
#include <Stela/System/InstallDir.h>
#include <Stela/System/GetCwd.h>
#include <fstream>
#include <math.h>


#define PREPARG_FILE <args.h>
#include <PrepArg/usage.h>

int main( int argc, char **argv ) {
    // args
    #include <PrepArg/declarations.h>
    #include <PrepArg/parse.h>
    if ( beg_files < 0 )
        return usage( argv[ 0 ], "Please specify an input file", 2 );
    bool add_base_files = not ( disp_lexems or disp_tokens );

    ParsingContext::GlobalVariables gv;
    ParsingContext pc( gv );

    // predefs
    pc.add_inc_path( base_met_files );
    if ( add_base_files and false ) {
        pc.parse( String( base_met_files ) + "/base.met", "" );
        PRINT( disp_inst_g        );
        PRINT( disp_inst_g_wo_phi );
        PRINT( code_for_class );
        PRINT( output );
    }

    // input files
    String cwd = get_cwd();
    for( int i = beg_files; i < argc; ++i )
        pc.parse( argv[ i ], cwd );
    for( ParsingContext::NamedVar &nv : pc.variables )
        PRINT( nv.expr );
    if ( gv.error_list )
        return gv.error_list;

    return 0;
}



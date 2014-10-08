/**
  main file for the stela interpreter / compiler
*/
#include <Stela/System/InstallDir.h>
#include <Stela/Conv/ConvContext.h>
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

    // global conversion context
    ConvContext cci;
    cc = &cci;

    // predefs
    cc->add_inc_path( base_met_files );
    if ( add_base_files and false ) {
        cc->parse( String( base_met_files ) + "/base.met", "" );
        PRINT( disp_inst_g        );
        PRINT( disp_inst_g_wo_phi );
    }

    // input files
    String cwd = get_cwd();
    for( int i = beg_files; i < argc; ++i )
        cc->parse( argv[ i ], cwd );
    if ( cc->error_list )
        return cc->error_list;

    if ( code_for_class ) {
        if ( not output ) {
            std::cerr << cc->error_list.add( "code_for_class needs an output filename (-o ...)" );
            return 1;
        }
        std::ofstream out_file( output );
        cc->out_code_for_class( out_file, code_for_class );
    }

    return 0;
}



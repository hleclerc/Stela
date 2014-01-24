/**
  main file for the stela interpreter / compiler
*/
#include <Stela/Interpreter/Interpreter.h>
#include <Stela/Compilers/CppCompiler.h>
#include <Stela/System/InstallDir.h>
#include <Stela/Met/IrWriter.h>
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

    // particular case
    if ( disp_tokens or disp_lexems or disp_inst_g or disp_inst_g_wo_phi )
        TODO;

    // input files
    Vec<String> input_files;
    if ( add_base_files and false )
        input_files << String( base_met_files ) + "/base.met";
    for( int i = beg_files; i < argc; ++i )
        input_files << argv[ i ];

    // context
    ErrorList e;
    Interpreter ip( e );
    ip.add_inc_path( base_met_files );
    ip.set_argc( argc - beg_files );
    ip.set_argv( argv + beg_files );

    // parse
    for( int i = 0; i < input_files.size(); ++i )
        ip.import( input_files[ i ] );
    Vec<ConstPtr<Inst> > outputs = ip.get_outputs();

    // compile
    CppCompiler cr;
    for( int i = 0; i < outputs.size(); ++i )
        cr << outputs[ i ];
    cr.exec();

    return e;
}


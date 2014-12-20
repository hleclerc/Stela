/**
  main file for the stela interpreter / compiler
*/
#include <Stela/Ssa/ParsingContext.h>
#include <Stela/Ssa/Type.h>
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

    GlobalVariables gv;
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
    //for( ParsingContext::NamedVar &nv : pc.variables )
    //    PRINT( nv.expr );
    if ( gv.error_list )
        return gv.error_list;

    //SARG(  0 , code_for_class    , "optionnal name of a class for which we want code", 0 );
    //SARG( 'o', output            , "", 0 );
    // objectif: sortir du code pour les classes
    if ( code_for_class ) {
        Expr e = pc.get_var( code_for_class );
        if ( not e ) {
            std::cerr << "Impossible to find var " << code_for_class << std::endl;
            return 1;
        }
        Expr res = pc.apply( e, 0, 0, 0, 0, 0, ParsingContext::APPLY_MODE_PARTIAL_INST );
        res->ptype()->parse();
        PRINT( *res->ptype() );

        //Type *c = e.get_type_for( Vec<Expr>() );
    }


    return 0;
}



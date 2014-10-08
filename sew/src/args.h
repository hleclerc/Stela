DESCRIPTION( "the stela interpreter" );

BARG(  0 , disp_tokens       , "display token stream", false );
BARG(  0 , disp_lexems       , "display lexem graph" , false );
BARG(  0 , disp_inst_g       , "display instruction graph", false );
BARG(  0 , disp_inst_g_wo_phi, "display instruction graph without phi nodes", false );
SARG(  0 , base_met_files    , "directory of of the base .met files", INSTALL_DIR "/met" );
SARG(  0 , code_for_class    , "optionnal name of a class for which we want code", 0 );
SARG( 'o', output            , "", 0 );
EARG( beg_files, "File to compile" );

DESCRIPTION( "the stela interpreter" );

BARG(  0 , disp_tokens       , "display token stream", false );
BARG(  0 , disp_lexems       , "display lexem graph" , false );
BARG(  0 , disp_inst_g       , "display instruction graph", false );
BARG(  0 , disp_inst_g_wo_phi, "display instruction graph without phi nodes", false );
SARG(  0 , base_met_files    , "directory of of the base .met files", INSTALL_DIR "/met" );
EARG( beg_files, "File to compile" );

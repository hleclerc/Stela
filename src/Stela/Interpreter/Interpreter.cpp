#include "../System/ReadFile.h"
#include "../System/ToDel.h"
#include "../Met/IrWriter.h"
#include "../Met/Lexer.h"
#include "Interpreter.h"
#include "SourceFile.h"
#include "Scope.h"
#include "Cst.h"

NstrCor glob_nstr_cor;

Interpreter::Interpreter( ErrorList &error_list ) :
    #define DECL_BT( T ) type_##T( this, &type_Type ),
    #include "DeclBaseClass.h"
    #undef DECL_BT
    #define DECL_BT( T ) class_##T( this, &type_Class ),
    #include "DeclBaseClass.h"
    #include "DeclParmClass.h"
    #undef DECL_BT
    error_var( this, &type_Error ),
    error_list( error_list ) {

    main_scope = 0;
}

Interpreter::~Interpreter(){
    delete main_scope;

    for( int i = obj_to_delete.size() - 1; i >= 0; --i )
        delete obj_to_delete[ i ];
}

void Interpreter::import( String filename ) {
    if ( already_imported( filename ) )
        return;

    // -> source data
    ReadFile r( filename.c_str() );
    if ( not r )
        return disp_error( "Impossible to open " + filename );

    // -> lexical data
    Lexer l( error_list );
    l.parse( r.data, filename.c_str() );
    if ( error_list )
        return;

    //    if ( disp_lexems ) {
    //        l.display();
    //        return 0;
    //    }

    // -> binary stream
    IrWriter t( error_list );
    t.parse( l.root() );
    if ( error_list )
        return;
    ST bin_size = t.size_of_binary_data();

    // -> make a new SourceFile
    Vec<PI8> sf_data;
    SourceFile::make_dat( sf_data, bin_size, filename );
    t.copy_binary_data_to( const_cast<PI8 *>( SourceFile( sf_data.ptr() ).bin_data() ) );
    Var *sf = sourcefiles.push_back( this, &type_SourceFile, cst( sf_data ) );

    //    if ( disp_tokens ) {
    //        //for( int i = 0; i < size; ++i )
    //        //    std::cout << std::hex << std::uppercase << (int)tok[ i ] << ' ';
    //        //std::cout << std::dec << std::endl;
    //        std::cout << IrDisplayer( tok.ptr() );
    //        return 0;
    //    }

    // -> virtual machine

    // -> execute
    if ( not main_scope )
        main_scope = new Scope( this, 0 );

    main_scope->parse( sf, tok_data_of( sf ) );
}

void Interpreter::add_inc_path( String path ) {
    inc_paths << path;
}

void Interpreter::set_argc( int argc ) {
    this->argc = argc;
}

void Interpreter::set_argv( char **argv ) {
    this->argv = argv;
}

ErrorList::Error &Interpreter::make_error( String msg, const Var *sf, int off, Scope *sc, bool warn ) {
    ErrorList::Error &res = error_list.add( msg );
    if ( sf )
        res.ac( SourceFile( sf->cst_data() ).filename(), off );
    for( Scope *s = sc; s; s = s->caller )
        if ( s->instantiated_from_sf )
            res.ac( SourceFile( s->instantiated_from_sf->cst_data() ).filename(), s->instantiated_from_off );

    return res;
}

void Interpreter::disp_error( String msg, const Var *sf, int off, Scope *sc, bool warn ) {
    std::cerr << make_error( msg, sf, off, sc, warn );
}

bool Interpreter::already_imported( String filename ) {
    for( int i = 0; i < sourcefiles.size(); ++i ) {
        TODO;
    }
    return false;
}

const PI8 *Interpreter::tok_data_of( const Var *sf ) {
    SfInfo &res = sf_info_of( sf );
    return res.tok_data;
}

SfInfo &Interpreter::sf_info_of( const Var *sf ) {
    SourceFile nsf( sf->cst_data() );
    const PI8 *sf_bin_data = nsf.bin_data();

    auto iter = sf_info_map.find( sf_bin_data );
    if ( iter != sf_info_map.end() )
        return iter->second;
    SfInfo &res = sf_info_map[ sf_bin_data ];

    // nstr_cor
    BinStreamReader bsr( sf_bin_data );
    while ( true ) {
        ST off = bsr.read_positive_integer();
        if ( not off )
            break;
        res.nstr_cor << glob_nstr_cor.num( String( bsr.ptr, bsr.ptr + off ) );
        bsr.ptr += off;
    }

    // tok_data
    res.tok_data = bsr.ptr;

    return res;
}



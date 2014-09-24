#include <boost/filesystem/operations.hpp> // includes boost/filesystem/path.hpp
#include "BinStreamWriter.h"
#include "BinStreamReader.h"
#include "BufStreamReader.h"
#include <fstream>
#ifdef METIL_COMP_DIRECTIVE
#pragma lib_name boost_system
#pragma lib_name boost_filesystem
#endif // METIL_COMP_DIRECTIVE
using namespace Celo;


/*/
- Stockage d'un Type *, puis de place pour pointeur et/ou données

*/
typedef long Inode;
struct Type;

struct Item {
    Type *type;
    char *raw_data; ///< used for updates
    char *compressed; ///< used for storage and transmission
};


struct Type {
    Type() {
        get_inode = 0;
    }
    typedef Inode TF_get_inode( Item *item, Ptr<Buffer> buf_filename, int off_filename );

    TF_get_inode *get_inode; ///< e.g. for Directory. Allows to find a file.
};

/**
  data =
    int   size
    int   rese
    void *data

  Pb: stockage sur le disque.
    On pourrait faire une méthode de stockage/transfert, avec l'idée que c'est différent de ce qu'on
      a en mémoire. Ça peut être intéressant pour les mises à jour complexes genre images
    Prop 1 : un pointeur sur la version compressé (si à jour) + un pointeur pour la version "normale"

*/
Type type_Directory;

struct Database {
    Database() : cur_inode( 1 ) {
    }

    Inode get_new_inode_async() {
        return cur_inode++;
    }

    Inode cur_inode;
};
Database db;

void get_files( std::vector<std::pair<std::string,int> > &res, std::string directory = "." ) {
    boost::filesystem::path dir_path( directory );
    for ( boost::filesystem::directory_iterator itr( dir_path ), end_itr; itr != end_itr; ++itr ) {
        if ( boost::filesystem::is_directory( *itr ) )
            get_files( res, itr->path().string() );
        else
            res.push_back( std::pair<std::string,int>( itr->path().string(), boost::filesystem::file_size( itr->path() ) ) );
    }
}

void bootstrap( const char *directory = "." ) {
    std::vector<std::pair<std::string,int> > files;
    get_files( files, directory );
    for( unsigned i = 0; i < files.size(); ++i ) {
        PRINT( files[ i ].first );
        PRINT( files[ i ].second );
        std::ifstream fin( files[ i ].first.c_str() );
        char data[ files[ i ].second + 1 ];
        fin.read( data, files[ i ].second );
        data[ files[ i ].second ] = 0;
        PRINT( data );
        //
    }
}



int main() {
    bootstrap( "tests" );
}



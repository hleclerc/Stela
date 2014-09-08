// #include "../ext/SocaDB/src/SocaDB/System/BinStreamWriter.h"
#include "BinStreamWriter.h"
#include "BinStreamReader.h"
#include "BufStreamReader.h"
using namespace Celo;


/**
  Stockage des répertoires.

  Prop 1: un arbre compressé basé sur les lettre par ordre chronologique.
    -> permet d'avancer dans l'arbre en même temps qu'on lit les données
    -> pas terrible pour les fichiers longs

  Prop 2: une table de hashage de taille dynamique

  Prop 3: au choix a posteriori, on passe par des méthodes virtuelles
    -> sol 1: prend en arg buffer + offset
*/

/**
  Directory =
    string size
    string data
    inode
    ...
*/

//typedef int Inode;

//struct Type {
//    typedef Inode T_GetInode( Ptr<Buffer> b, int off, int len );
//};


//struct Item {
//    Type *type;
//    void *data;
//};

//void test_read( const char *url ) {
//    Ptr<Buffer> b = new Buffer;
//    memcpy( b->data, url, strlen( url ) );
//    b->used = strlen( url );


//}

void basic_rw_test() {
    void *ptr = malloc( 1000 );
    BinStreamWriter bw( ptr );
    bw.write_unsigned(    1 );
    bw.write_unsigned(   10 );
    bw.write_unsigned(  100 );
    bw.write_unsigned( 1000 );
    bw.write_signed(     1 );
    bw.write_signed( -  10 );
    bw.write_signed(   100 );
    bw.write_signed( -1000 );
    bw.write_signed( -1024 * 1024 );

    BinStreamReader br( ptr, bw.ptr );
    for( int o = 0; o < 4; ++o )
        PRINT( int( br.read_unsigned() ) );
    for( int o = 0; o < 5; ++o )
        PRINT( int( br.read_signed() ) );
}



/*
  Directory:
    string size (bytes)
    string data
    inode
    ...
    (ended by a size == 0)

  OU

  Directory:
    string size (bytes)
    compressed string data
    inode
    ...
    (ended by a size == 0)

  Rq: on pourrait stocker les noms de répertoire sous forme compressée
  -> les GET seraient sous forme non compressée
  -> Rq: les noms sont en général long, c'est sans doute mieux de faire taille (compressée) + donnée
*/
void make_dir( BinStreamWriter &bw ) {
    std::string names[] = { "toto", "ex", "a" };
    for( int i = 0; i < 3; ++i ) {
        bw.write_unsigned( names[ i ].size() ); // string size
        bw.write( names[ i ].data(), names[ i ].size() ); // string data
        bw.write_unsigned( i ); // inode
    }
    bw.write_unsigned( 0 ); // end of directory data
}

/*
str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;="
str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789. "

*/
void write_url( BinStreamWriter &bw, std::string url ) {

}

int get_inode( void *beg_dir_data, void *end_dir_data, Ptr<Buffer> buf_filename, int off_filename ) {
    BinStreamReader br( beg_dir_data, end_dir_data );
    BufStreamReader br( buf_filename, off_filename );
    while ( true ) {
        int s = br.read_unsigned();

    }
    return 0;
}

int main() {
    //    basic_rw_test();

    void *dir = malloc( 1000 );
    BinStreamWriter bw( dir );
    make_dir( bw );

    Ptr<Buffer> fna = new Buffer;
    memcpy( fna->data, "toto", 4  );
    fna->used = 4;

    PRINT( get_inode( dir, bw.ptr, fna, 0 ) );
}



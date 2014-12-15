#include "../PI8.h"

/**
  SocaDB object.
*/
struct SO {
    SO( char *ptr, int type_num, int type_inst ) : ptr( ptr ), type_num( type_num ), type_inst( type_inst ) {
    }
    char *ptr;
    int   type_num;
    int   type_inst;
};

/**
*/
struct PI8 {
    PI8( SO __par, char  *__ptr, char __bof ) : __par( __par ), __ptr( ptr ), __bof( __bof ) {
    }
    int __size() const {
        return 8;
    }
    int __alig() const {
        return 8;
    }
    //operator PI8() const {
    //    return *reinterpret_cast<const PI8 *>( __ptr ); // +- bof
    //}
    SO     __par;
    char  *__ptr;
    char   __bof;
};

namespace SocaDB {
int __size( PI8 ) const {
    return 8;
}
int __alig() const {
    return 8;
}
}

/**
  Workflow pour les clients
    SB::Directory dir( "/home" );
    SB::User john( dir.find( "john" ) );
    PRINT( john.surname() );
    john.surname() = "yuki"; ///< -> envoie à la db
    dir

  Workflow côté serveur
    utilisation des fonctions de communication
      apply_patch...
    + utilisation ponctuelle des fonctions client (par exemple du find sur un répértoire)
*/
int main() {
    PI8 a, b;
    PRINT( a + b );

    // allocation sur la pile -> va poser de gros soucis pour les resize, etc...
    // prop: on ne l'autorise pas

}

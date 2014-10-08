#include "../PI8.h"

/**
*/
template<class T>
struct Ref {};

/**
*/
struct Model {
    Model( char  *__ptr, Model *__par, char __bof ) : __ptr( __ptr ), __par( __par ), __bof( __bof ) {
    }

    char  *__ptr;
    Model *__par;
    char   __bof;
};

/**
*/
struct PI8 {};

/**
  Rq: on ne pourra pas générer du code pour des méthodes libres, à moins de spécifier le type des arguments en entrée
  -> type model ou pas type model ?
*/
template<class T>
struct Ref<PI8> : public Model {
    Ref( char  *__ptr, Model *__par, char __bof ) : Model( __ptr, __par, __bof ) {
    }
    int __size() const {
        return 8;
    }
    int __alig() const {
        return 8;
    }
    operator PI8() const {
        return *reinterpret_cast<const PI8 *>( __ptr ); // +- bof
    }
};

int main() {
    PI8 a, b;
    PRINT( a + b );

    // allocation sur la pile -> va poser de gros soucis pour les resize, etc...
    // faut-il
    NEW_CUnsigned( a, 17 );


}

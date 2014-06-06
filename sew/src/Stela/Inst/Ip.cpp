#include "Type.h"
#include "Ip.h"

Ip *ip;

Ip::Ip() {
    #define DECL_BT( T ) type_##T = new Type;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    type_SI32->_len = 32;
}

Ip::~Ip() {
    #define DECL_BT( T ) delete type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT
}

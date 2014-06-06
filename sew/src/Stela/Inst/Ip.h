#ifndef IP_H
#define IP_H

struct Type;

/**
*/
class Ip {
public:
    Ip();
    ~Ip();

    #define DECL_BT( T ) Type *type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT
};

extern Ip *ip;

#endif // IP_H

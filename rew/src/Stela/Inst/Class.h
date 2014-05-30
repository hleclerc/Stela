#ifndef CLASS_H
#define CLASS_H

#include "Callable.h"
class Type;

/**
*/
class Class : public Callable {
public:
    Class();

    Type *type_for( const Vec<Var> &args );

    Vec<Type *> types;
};

#endif // CLASS_H

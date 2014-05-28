#ifndef CLASS_H
#define CLASS_H

#include "Callable.h"
class Type;

/**
*/
class Class : public Callable {
public:
    Class();

    Type *find_type( const Vec<Var> &args );

    Vec<Type *> types;
};

#endif // CLASS_H

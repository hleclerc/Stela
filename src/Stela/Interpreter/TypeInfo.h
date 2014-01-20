#ifndef TYPEINFO_H
#define TYPEINFO_H

#include "../System/Vec.h"
#include "Var.h"

/**
*/
class TypeInfo {
public:
    TypeInfo();

    Vec<Var>  parameters;
    Var       type_var;
    TypeInfo *prev;
};

#endif // TYPEINFO_H

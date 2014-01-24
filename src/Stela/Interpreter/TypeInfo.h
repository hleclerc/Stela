#ifndef TYPEINFO_H
#define TYPEINFO_H

#include "../System/Vec.h"
#include "Var.h"
class ClassInfo;

/**
*/
class TypeInfo {
public:
    TypeInfo( ClassInfo *orig );

    Vec<Var>   parameters;
    ClassInfo *orig;
    Var        var;
    TypeInfo  *prev;
};

#endif // TYPEINFO_H

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

    // input
    Vec<Var>   parameters;
    ClassInfo *orig;
    Var        var;

    //
    bool       parsed;

    //
    TypeInfo  *prev; ///< prev type in ClassInfo
};

#endif // TYPEINFO_H

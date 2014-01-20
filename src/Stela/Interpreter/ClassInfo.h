#ifndef CLASSINFO_H
#define CLASSINFO_H

#include "TypeInfo.h"

/**
*/
class ClassInfo {
public:
    ClassInfo();

    TypeInfo *last;
    Expr      expr;
};

#endif // CLASSINFO_H

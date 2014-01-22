#ifndef CLASSINFO_H
#define CLASSINFO_H

#include "TypeInfo.h"

/**
*/
class ClassInfo {
public:
    ClassInfo();
    ~ClassInfo();

    TypeInfo *last;
    Expr      expr;
};

#endif // CLASSINFO_H

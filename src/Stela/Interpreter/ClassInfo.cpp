#include "ClassInfo.h"

ClassInfo::ClassInfo() : last( 0 ) {
}

ClassInfo::~ClassInfo() {
    for( TypeInfo *t = last; t;  ) {
        TypeInfo *l = t;
        t = t->prev;
        delete l;
    }
}


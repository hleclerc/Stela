#ifndef CALLABLE_H
#define CALLABLE_H

#include "../System/BinStreamReader.h"
#include "Var.h"
class SourceFile;

/**
*/
class Callable {
public:
    Callable();

    virtual void read_bin( BinStreamReader &bin );

    int name;

    SourceFile *sf;
    int off; ///< in sourcefile
};

#endif // CALLABLE_H

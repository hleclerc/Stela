#ifndef SOURCEFILE_H
#define SOURCEFILE_H

#include "../System/Stream.h"
#include "../System/Vec.h"

/**
*/
class SourceFile {
public:
    SourceFile();

    Vec<PI8> tok_data;
    Vec<int> cor_str;
    String name;
};

#endif // SOURCEFILE_H

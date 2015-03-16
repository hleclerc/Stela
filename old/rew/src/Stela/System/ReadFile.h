#ifndef READFILE_H
#define READFILE_H

#include "TypeConfig.h"

/**
  Read file content and add a \0 at the end.

  data is freed by the destructor of ReadFile (unless it has been set to 0)
*/
struct ReadFile {
    ReadFile( const char *name );
    ~ReadFile();

    operator bool() const;

    char *data;
    ST    size;
};


#endif // READFILE_H

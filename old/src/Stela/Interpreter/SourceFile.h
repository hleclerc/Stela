#ifndef SOURCEFILE_H
#define SOURCEFILE_H

#include "../System/Stream.h"
#include "Var.h"

/**
  interface for SourceFile data (stored in Var objects)
  basically contains
    - int( size_of_tok_data )
    - tok_data
    - int( size_of_filename )
    - filename (\0 ended)
*/
class SourceFile {
public:
    static void prep_dat( Vec<PI8> &data, ST bin_size, String filename ); ///< partial

    SourceFile( const PI8 *ptr );

    const char *filename() const;
    int         bin_size() const;
    int         str_size() const; ///< size of filename string
    const PI8  *bin_data() const; ///< at the beginning: nstring cor table
    String      dir() const;
    int         tot_size() const;

protected:
    const PI8 *ptr;
};

#endif // SOURCEFILE_H

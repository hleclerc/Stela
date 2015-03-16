#include "FileExists.h"
#include <sys/stat.h> // mkdir...

bool file_exists( const char *f ) {
    struct stat stat_file;
    return ( stat( f, &stat_file ) == 0 );
}

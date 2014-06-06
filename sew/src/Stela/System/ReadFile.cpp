#include "ReadFile.h"
#include <fstream>

ReadFile::ReadFile( const char *name ) {
    std::ifstream f( name );
    if ( not f ) {
        data = new char[ 1 ];
        data[ 0 ] = 0;
        size = -1;
        return;
    }

    f.seekg( 0, std::ios::end );
    size = f.tellg();
    f.seekg( 0, std::ios::beg );

    data = new char[ size + 1 ];
    f.read( data, size );
    data[ size ] = 0;
}

ReadFile::~ReadFile() {
    delete [] data;
}

ReadFile::operator bool() const {
    return size >= 0;
}

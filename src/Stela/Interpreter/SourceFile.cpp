#include "../System/BinStreamReader.h"
#include "Interpreter.h"
#include "SourceFile.h"

void SourceFile::make_dat( Vec<PI8> &data, ST bin_size, String filename ) {
    data.resize( sizeof( int ) + bin_size + sizeof( int ) + filename.size() + 1 );
    reinterpret_cast<int &>( data[ 0 ] ) = bin_size;
    // -> memcpy( data.ptr() + sizeof( int ), ..., bin_size );
    reinterpret_cast<int &>( data[ sizeof( int ) + bin_size ] ) = filename.size();
    memcpy( data.ptr() + sizeof( int ) + bin_size + sizeof( int ), filename.c_str(), filename.size() + 1 );
}

SourceFile::SourceFile( const PI8 *ptr ) : ptr( ptr ) {
}

const char *SourceFile::filename() const{
    return reinterpret_cast<const char *>( ptr + sizeof( int ) + bin_size() + sizeof( int ) );
}

int SourceFile::bin_size() const {
    return *reinterpret_cast<const int *>( ptr );
}

const PI8 *SourceFile::bin_data() const {
    return ptr + sizeof( int );
}

String SourceFile::dir() const {
    String f = filename();
    ST pos = f.rfind( '/' );
    if ( pos < 0 )
        return String();
    return String( f.begin(), f.begin() + pos );
}


#include "Memcpy.h"
#include "Assert.h"
#include <string.h>

void memcpy_bit( void *dst, int off_dst, const void *src, int off_src, int len ) {
    if ( int o = off_dst / 8 ) return memcpy_bit( (PI8*)dst + o, off_dst % 8, (PI8*)src, off_src, len );
    if ( int o = off_src / 8 ) return memcpy_bit( (PI8*)dst, off_dst, (PI8*)src + o, off_src % 8, len );

    if ( off_src % 8 or off_src % 8 )
        TODO;
    else
        memcpy( dst, src, ( len + 7 ) / 8 );
}

void memcpy_bit( void *dst, const void *src, int len ) {
    return memcpy_bit( dst, 0, src, 0, len );
}
